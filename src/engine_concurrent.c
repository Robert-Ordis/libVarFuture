/**
 *	\file		engine_threadpool.c
 *	\brief		エンジン部分のうち、並列処理関連の雑多な実装についてまとめたもの
 *	
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "varfuture/private/engine/trigger.h"
#include "varfuture/private/body.h"
#include "varfuture/private/engine/threadpool.h"
#include "varfuture/private/engine/concurrent_api.h"
#include "varfuture/private/queue.h"
#include "varfuture/private/queue_api.h"
#include "varfuture/varfuture_depend.h"

//グローバルエリアでのロックの素材
static varfuture_lock_t	global_lock = VARFUTURE_LOCK_GLOBAL_INIT;
//future待機関連の条件変数
static varfuture_cond_t	global_cond;
static varfuture_condattr_t	global_condattr;
//タスクキュー待機関連の条件変数
static varfuture_cond_t	global_quemod_cond = VARFUTURE_COND_GLOBAL_INIT;

//グローバルエリアのトリガー（ロックは同じ）
static struct varfuture_trigger_s	global_trigger;			//future貸し出し用
static struct varfuture_trigger_s	global_quemod_trigger;	//queue貸し出し用

//スレッドプール
static varfuture_threadpool_t	*global_tpool;

//キュー
static varfuture_queue_t		global_queue;

//スレッドプールの数
static int tp_size = 0;

//次取得するスレッドプールの番号
//static int tp_next = 0;

static inline int ptr_to_index(intptr_t ptr){
	int ret;
	if(sizeof(intptr_t) == sizeof(int64_t)){
		uint64_t x = ptr;
		x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
		x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
		x = x ^ (x >> 31);
		ret = (int)x;
	}
	else{
		uint32_t x = ptr;
		x ^= x >> 17;
		x *= (0xed5ad4bb);
		x ^= x >> 11;
		x *= (0xac4c1b51);
		x ^= x >> 15;
		x *= (0x31848bab);
		x ^= x >> 14;
		ret = (int)x;
	}
	ret = ret % tp_size;
	if(ret < 0){
		ret *= -1;
	}
	return ret;
}

/**
 *	\fn			varfuture_concurrent_init
 *	\brief		初期化。それに伴ってバックグラウンドスレッドの起動
 *	\param		(int)tp_num		:スレッド数
 *	\return	(int)起動できた数。-1, -2でcond初期化エラー。-3でmalloc/-4以下でスレッド起動エラー。かつerrno格納。
 *	\remarks	tp_numは1以下の場合では強制的にデフォルト数起動とする
 */
int	varfuture_concurrent_init(int tp_num){
	int err_num;
	int	ret_num = 0;
	int	i;
	
	if(tp_num < 2){
		//現在、スレッド数は2以上を強制したい。
		tp_num = 4;
	}
	
	varfuture_lock_acquire(&global_lock, &err_num);{
		do{
			if(tp_size){
				break;
			}
			
			//まずは条件変数の初期化
			varfuture_cond_init(&global_cond, &global_condattr, &err_num, &ret_num);
			if(ret_num < 0){
				break;
			}
			
			//トリガーのアサイン。以降はこのトリガーをベースに動いていく
			varfuture_trigger_assign(&global_trigger, &global_lock, &global_cond);
			varfuture_trigger_assign(&global_quemod_trigger, &global_lock, &global_quemod_cond);
			
			//次にメモリ確保
			global_tpool = (varfuture_threadpool_t *)malloc(sizeof(varfuture_threadpool_t) * tp_num);
			if(global_tpool == NULL){
				//malloc起動失敗
				err_num = errno;
				ret_num = -3;
				break;
			}
			
			//キューの初期化
			varfuture_queue_init(&global_queue, &global_trigger, &global_quemod_trigger);
			
			//次にスレッドプールの初期化と起動
			for(i = 0; i < tp_num; i++){
				if(varfuture_threadpool_init(&global_tpool[i], &global_queue) < 0){
					err_num = errno;
					ret_num = -4;
					break;
				}
			}
			
			for(i = 0; i < tp_num; i++){
				if(varfuture_threadpool_boot(&global_tpool[i]) < 0){
					err_num = errno;
					ret_num = -5;
					break;
				}
			}
			
			
		}while(0);
		switch(ret_num){
		case -5:
			for(i = 0; i < tp_num; i++){
				varfuture_threadpool_down(&global_tpool[i]);
			}
		case -4:
			free(global_tpool);
		case -3:
			break;
		case 0:
			tp_size = tp_num;
			break;
		default:
			break;
		}
	}varfuture_lock_release(&global_lock, &err_num);
	
	errno = err_num;
	return ret_num;
}

/**
 *	\fn			varfuture_concurrent_get_threadpool_queue
 *	\brief		バックグラウンドスレッドに関するキューを一つ取得する
 *	\return	(ptr)バックグラウンドスレッド用のキューのポインタ
 *	\remarks	ただし、バックグラウンドスレッドから命令された場合はNULLを返す
 *	\todo		スレッドの進行状況を見てビジーに陥りそうならisolateに誘導する
 */
varfuture_queue_t* varfuture_concurrent_get_threadpool_queue(){
	varfuture_queue_t *ret_queue = NULL;
	int err_num;
	int i;
	varfuture_lock_acquire(&global_lock, &err_num);{
		do{
			if(tp_size <= 0){
				break;
			}
			//起こしたスレッドプールから呼ばれた場合はデッドロックが怖いので返さない。
			for(i = 0; i < tp_size; i++){
				//printf("%s: check with num[%d]\n", __func__, i);
				//printf("%s: current[%p] vs tp[%p]\n", __func__, th, global_tpool[i].th);
				if(varfuture_threadpool_is_current_thread(&global_tpool[i])){
					break;
				}
			}
			if(i != tp_size){
				//printf("%s: from tp thread\n", __func__);
				break;
			}
			//printf("%s: not from tp thread\n", __func__);
			ret_queue = &global_queue;
		}while(0);
	}varfuture_lock_release(&global_lock, &err_num);
	
	return ret_queue;
}


/**
 *	\fn			varfuture_concurrent_get_isolate_trigger
 *	\brief		スレッドプールとは別の独立トリガーをもらい受ける
 *	\return	(ptr)独立動作用のトリガー。多分全部一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_isolate_trigger(){
	return &global_trigger;
}

/**
 *	\fn			varfuture_concurrent_get_queue_modify_trigger
 *	\brief		タスクキュー処理用のプール処理トリガーをもらう
 *	\return	(ptr)グローバル用のトリガー。malloc無しで動かす都合上多分ロックは一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_queue_modify_trigger(){
	return &global_quemod_trigger;
}

/**
 *	\fn			varfuture_concurrent_get_queue_future_trigger
 *	\brief		タスクキュー処理用のfuture通知トリガーをもらう
 *	\return	(ptr)グローバル用のトリガー。malloc無しで動かす都合上多分ロックは一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_queue_future_trigger(){
	return &global_trigger;
}

/**
 *	\fn			varfuture_concurrent_invoke_isolate
 *	\brief		独立future用に１つ起動する
 *	\return	(int)0で成功。負値で失敗（errnoにそれっぽいこと書く）
 */
int	varfuture_concurrent_invoke_isolate(struct varfuture_body_s *future){
	varfuture_thread_t thread;
	int ret;
	if(future == NULL){
		return -1;
	}
#ifdef	VARFUTURE_USE_PTHREAD
	//printf("%s[%p]: invoke isolate thread\n", __func__, future);
	if((ret = pthread_create(&thread, NULL, future->func, future)) < 0){
		return ret;
	}
	pthread_detach(thread);
#endif	/* !VARFUTURE_USE_PTHREAD */
	return ret;
}
/**
 *	\fn			varfuture_concurrent_trigger_acquire
 *	\brief		指定のトリガーをもってロックを獲得する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_acquire(struct varfuture_trigger_s *trigger){
	int err_num = 0;
	//printf("%s[%p]: \n", __func__, trigger);
	varfuture_trigger_acquire(trigger, &err_num);
	errno = err_num;
	return !err_num ? 0:-1;
}

/**
 *	\fn			varfuture_concurrent_trigger_release
 *	\brief		指定のトリガーをもってロックを解放する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_release(struct varfuture_trigger_s *trigger){
	int err_num = 0;
	varfuture_trigger_release(trigger, &err_num);
	//printf("%s[%p]: \n", __func__, trigger);
	errno = err_num;
	return !err_num ? 0:-1;
}

/**
 *	\fn			varfuture_concurrent_trigger_wait
 *	\brief		指定のトリガーをもって信号を待機する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_wait(struct varfuture_trigger_s *trigger){
	int err_num = 0;
	varfuture_trigger_wait(trigger, &err_num);
	errno = err_num;
	return !err_num ? 0:-1;
}

/**
 *	\fn			varfuture_concurrent_trigger_timedwait
 *	\brief		指定のトリガーをもって信号を待機する
 *	\param		(ptr)trigger			:トリガー。全部とまでは言わないけど大体同じ
 *	\param		(int64_t)tout_millis	:街時刻
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_timedwait(struct varfuture_trigger_s *trigger, int64_t tout_millis){
	int err_num = 0;
	struct timeval dbgt;
	gettimeofday(&dbgt, NULL);
	//printf("%s[%p]: wait in %lld, %lld\n", __func__, trigger, dbgt.tv_sec, dbgt.tv_usec);
	varfuture_trigger_timedwait(trigger, tout_millis, &err_num);
	//gettimeofday(&dbgt, NULL);
	//printf("%s[%p]: wait broken in %lld, %lld\n", __func__, trigger, dbgt.tv_sec, dbgt.tv_usec);
	//printf("%s[%p]: err_num=%d\n", __func__, trigger, err_num);
	errno = err_num;
	return !err_num ? 0:-1;
}

/**
 *	\fn			varfuture_concurrent_trigger_signal
 *	\brief		指定のトリガーをもって待機している人誰か一人起こす
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_signal(struct varfuture_trigger_s *trigger){
	int err_num = 0;
	varfuture_trigger_signal(trigger, &err_num);
	errno = err_num;
	return !err_num ? 0:-1;
}

/**
 *	\fn			varfuture_concurrent_trigger_notify
 *	\brief		指定のトリガー（の条件変数）で待機してる子全員起こす
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_broadcast(struct varfuture_trigger_s *trigger){
	int err_num = 0;
	varfuture_trigger_broadcast(trigger, &err_num);
	errno = err_num;
	return !err_num ? 0:-1;
}
