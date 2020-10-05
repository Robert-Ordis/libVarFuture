#include "./include/varfuture/private/body.h"
#include "./include/varfuture/varfuture_attr.h"
#include "./include/varfuture/void_future.h"
#include "./include/varfuture/private/engine/concurrent_api.h"
#include "./include/varfuture/private/queue_api.h"
#include "./include/varfuture/private/inline_processes.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

//起動前検査
//マクロ化可能関数
static inline int void_future_before_exec_invoke_(void_future_t *future){
	
	int ret = 0;
	/*
	//動作中の状態で設定しようとしても駄目です
	switch(future->body.state){
	case varfuture_state_idle:
	case varfuture_state_abrt:
		break;
	default:
		ret = -1;
		break;
	}
	
	if(ret < 0){
		return ret;
	}
	
	//関数指定実行モードでない場合NGです
	switch(future->body.exec_mode){
	case varfuture_mode_taskpool:
	case varfuture_mode_isolate:
		break;
	default:
		ret = -2;
		break;
	}
	*/
	VARFUTURE_BODY_BEFORE_EXEC_(&(future->body), ret);
	return ret;
}

//起動（execモードのみ）
//マクロ化可能関数
static inline int void_future_exec_invoke_(void_future_t *future){
	
	int ret = 0;
	/*
	varfuture_queue_t *q = future->body.queue;
	
	//モード設定が終わったので実行予約に投げ込んでおく
	switch(future->body.exec_mode){
	case varfuture_mode_taskpool:
		if((ret = varfuture_queue_append(q, &(future->body))) != 0){
			errno = EINVAL;
		}
		break;
	case varfuture_mode_isolate:
		if((ret = varfuture_concurrent_invoke_isolate(&(future->body))) != 0){
			//スレッドの起動に失敗したらそのまま終わる。
			errno = ret;
			future->body.err_num = ret;
			future->body.state = varfuture_state_abrt;
		}
		break;
	default:
		break;
	}
	*/
	VARFUTURE_BODY_EXEC_INVOKE_(&(future->body), ret);
	return ret;
}

//futureタスクが執り行う関数の起点
//戻り値ごとに微妙に異なるボイラープレートなのでテンプレ化できればなお良し
static void *void_future_task(void *arg){
	
	void_future_t *future = (void_future_t *)arg;
	varfuture_body_t	*body = &(future->body);
	varfuture_trigger_t	*trigger = body->trigger;
	int err_num = 0;
	
	/*ステータス遷移。execに移行（＝これをもってキャンセルできなくなる）*/
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			/*何かを間違えてスレッド実行中にキャンセルが入ってもカバーしましょう*/
			if(body->state != varfuture_state_wait){
				err_num = ECANCELED;
				body->err_num = ECANCELED;
				break;
			}
			body->state = varfuture_state_exec;
		} while (0);
	}varfuture_concurrent_trigger_release(trigger);
	if(err_num != 0){
		return NULL;
	}
	errno = 0;
	/*実行モードによって関数の振り分け方を変える。*/
	switch(body->arg_mode){
	case 1:
		future->task.one(body->arg.s.first);
		break;
	case 2:
		future->task.two(body->arg.s.first, body->arg.s.second);
		break;
	case 3:
		future->task.three(body->arg.s.first, body->arg.s.second, body->arg.s.third);
		break;
	default:
		future->task.vector(body->arg.v.c, body->arg.v.v);
		break;
	}
	err_num = errno;
	
	/*ステータス遷移。idleへ移行（＝もう自由）*/
	varfuture_concurrent_trigger_acquire(trigger);{
		body->state = varfuture_state_idle;
		body->err_num = err_num;
		varfuture_concurrent_trigger_broadcast(trigger);
	}varfuture_concurrent_trigger_release(trigger);
	
	return NULL;
}


/**
 *	\fn			void_future_init_for_exec
 *	\brief		exec用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	スレッドプール内で呼んだら強制的にmode_isolateとなる
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_init_for_exec(void_future_t *future){
	static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;
	
	varfuture_queue_t *q = varfuture_concurrent_get_threadpool_queue();
	if(q == NULL){
		return void_future_init_for_isolate(future);
	}
	//future->body = VARFUTURE_BODY_INIT_MEMBERS;
	future->body = b;
	future->body.exec_mode = varfuture_mode_taskpool;
	future->body.queue = q;
	future->body.trigger = varfuture_queue_get_trigger(q);
	printf("%s[%p]: q is %p\n", __func__, future, q);
	return 0;
}

/**
 *	\fn			void_future_init_for_isolate
 *	\brief		独立スレッド用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_init_for_isolate(void_future_t *future){
	static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;
	
	future->body = b;
	future->body.exec_mode = varfuture_mode_isolate;
	future->body.queue = NULL;
	future->body.trigger = varfuture_concurrent_get_isolate_trigger();
	return 0;
}
	
//promise関連は未完成
/**
 *	\fn			void_future_init_for_promise
 *	\brief		promise用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\param		(void_promise_t *)		promise	:promiseオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_register_to_promise(void_future_t *future, void_promise_t *promise){
	static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;
	varfuture_trigger_t	*trigger = varfuture_queue_get_trigger(&(promise->queue));
	int ret = 0;
	printf("%s[%p, %p]: trigger is %p\n", __func__, future, promise, trigger);
	varfuture_concurrent_trigger_acquire(trigger);{
		future->body = b;
		future->body.exec_mode = varfuture_mode_promise;
		future->body.queue = &(promise->queue);
		
		if((ret = varfuture_queue_append(&(promise->queue), &(future->body))) == 0){
			future->body.trigger = trigger;
			future->body.state = varfuture_state_wait;
			printf("%s[%p, %p]: %p\n", __func__, future, promise, future->body.trigger);
		}
		
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_exec1
 *	\brief		引数１個のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_func1_cb)	func	:実際に行わせたい動作
 *	\param		(void*)					arg1	:任意引数
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_exec1(void_future_t *future, void_future_func1_cb func, void *arg1){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			/*起動前検査*/
			if((ret = void_future_before_exec_invoke_(future)) != 0){
				errno = EINVAL;
				break;
			}
			
			/*OKなので実行しましょうか*/
			
			/*ステート遷移：待機*/
			body->state = varfuture_state_wait;
			
			/*モード設定：引数１モードでいろいろ設定*/
			future->task.one = func;
			body->arg_mode = 1;
			body->arg.s.first = arg1;
			body->func = void_future_task;
			
			/*起動*/
			ret = void_future_exec_invoke_(future);
			
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_exec2
 *	\brief		引数２個のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_func2_cb)	func	:実際に行わせたい動作
 *	\param		(void*)					arg1	:任意引数
 *	\param		(void*)					arg2	:任意引数
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_exec2(void_future_t *future, void_future_func2_cb func, void *arg1, void *arg2){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			/*起動前検査*/
			if((ret = void_future_before_exec_invoke_(future)) != 0){
				errno = EINVAL;
				break;
			}
			
			/*OKなので実行しましょうか*/
			
			/*ステート遷移：待機*/
			body->state = varfuture_state_wait;
			
			/*モード設定：引数２モードでいろいろ設定*/
			future->task.two = func;
			body->arg_mode = 2;
			body->arg.s.first = arg1;
			body->arg.s.second = arg2;
			body->func = void_future_task;
			
			/*起動*/
			ret = void_future_exec_invoke_(future);
			
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_exec3
 *	\brief		引数３個のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_func3_cb)	func	:実際に行わせたい動作
 *	\param		(void*)					arg1	:任意引数
 *	\param		(void*)					arg2	:任意引数
 *	\param		(void*)					arg3	:任意引数
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_exec3(void_future_t *future, void_future_func3_cb func, void *arg1, void *arg2, void *arg3){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			/*起動前検査*/
			if((ret = void_future_before_exec_invoke_(future)) != 0){
				errno = EINVAL;
				break;
			}
			
			/*OKなので実行しましょうか*/
			
			/*ステート遷移：待機*/
			body->state = varfuture_state_wait;
			
			/*モード設定：引数３モードでいろいろ設定*/
			future->task.three = func;
			body->arg_mode = 3;
			body->arg.s.first = arg1;
			body->arg.s.second = arg2;
			body->arg.s.third = arg3;
			body->func = void_future_task;
			
			/*起動*/
			ret = void_future_exec_invoke_(future);
			
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_execv
 *	\brief		任意個引数のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_funcv_cb)	func	:実際に行わせたい動作
 *	\param		(int)					argc	:引数の個数
 *	\param		(void**)				argv	:引数配列
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_execv(void_future_t *future, void_future_funcv_cb func, int argc, void **argv){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			/*起動前検査*/
			if((ret = void_future_before_exec_invoke_(future)) != 0){
				errno = EINVAL;
				break;
			}
			
			/*OKなので実行しましょうか*/
			
			/*ステート遷移：待機*/
			body->state = varfuture_state_wait;
			
			/*モード設定：argvモードでいろいろ設定*/
			future->task.vector = func;
			body->arg_mode = -1;
			body->arg.v.c = argc;
			body->arg.v.v = argv;
			body->func = void_future_task;
			
			/*起動*/
			ret = void_future_exec_invoke_(future);
			
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_cancel
 *	\brief		予約済みのコールバック/プロミスをキャンセルする
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	0でキャンセル成功、マイナスで失敗
 *	\brief		idle及びwait状態で成功。exec状態では失敗しますよ。
 *	\brief		getで待機しているスレッドはたたき起こされます。
 *	\brief		成功した場合、get_errnoはECANCELED に設定されます
 *	\brief		promiseは入れても何も起こらなくなります
 */
int		void_future_cancel(void_future_t *future){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	varfuture_queue_t		*q;
	//void_promise_t			*promise;
	
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		printf("%s[%p]: canceling...\n", __func__, future);
		do{
			/*動作中の状態で設定しようとしても駄目です*/
			switch(body->state){
			case varfuture_state_idle:
			case varfuture_state_abrt:
				ret = -1;
				break;
			case varfuture_state_exec:
				ret = -2;
				
			default:
				break;
			}
			
			if(ret < 0){
				printf("%s[%p]: canceling->failure(%d)\n", __func__, future, ret);
				break;
			}
			
			
			/*各モードに見合ったキャンセル操作を行う*/
			/*ステート遷移：中断*/
			body->state = varfuture_state_abrt;
			body->err_num = ECANCELED;
			switch(body->exec_mode){
			case varfuture_mode_isolate:
				/*どこかに属してるわけでもないので特にやることない、と思う*/
				break;
			case varfuture_mode_taskpool:
			case varfuture_mode_promise:
				/*スレッドプールから除名*/
				q = body->queue;
				printf("%s[%p] remove from %p\n", __func__, future, q);
				ret = varfuture_queue_remove(q, body);
				break;
				break;
			default:
				ret = -2;
				break;
			}
			varfuture_concurrent_trigger_broadcast(trigger);
			
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
	
}

/**
 *	\fn			void_future_is_finished
 *	\brief		futureが何でもいいから終了したかどうかを返す
 *	\param		[in] (void\future_t *)	future	:futureオブジェクト
 *	\return	(int)							:終了してるなら1、それ以外で0
 *	\remarks	要するに、getで待たされるようでなければOKの感覚（ガバ）
 */
int		void_future_is_finished(void_future_t *future){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int ret = 0;
	
	varfuture_concurrent_trigger_acquire(trigger);{
		if(body->state != varfuture_state_exec && body->state != varfuture_state_wait){
			ret = 1;
		}
	}varfuture_concurrent_trigger_release(trigger);
	
	return ret;
}

/**
 *	\fn			void_future_get
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		[in] (void_future_t *)	future	:futureオブジェクト
 *	\param		[out](void*)			ret		:何もしない、示しがつかないので置いてるだけ
 *	\param		[out](int*)				err_ptr	:「処理内で」起こったerrnoの格納
 *	\return	(int)		:0で正常終了。それ以外で待機時エラーとし、下記詳細をerrnoに格納する。
 *	\error		EINTR		:割り込み
 *				ECANCELED	:キャンセルされた
 */
int		void_future_get(void_future_t *future, void *ret_ptr, int *err_ptr){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int err = 0;
	
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			//printf("%s[%p]: get, state is %d \n", __func__, future, body->state);
			if(body->state == varfuture_state_idle){
				/*idleの場合にはそのまま終わる。*/
			}
			else if(body->state == varfuture_state_abrt){
				/*abrtの場合、errnoを付ける*/
				err = ECANCELED;
			}
			else{
				/*waitやexecの場合はそのまま待機*/
				if(varfuture_concurrent_trigger_wait(trigger) < 0){
					err = errno;
				}
				continue;
			}
			if(ret_ptr != NULL){
				/* *ret_ptr = future->ret_val;*/
			}
			if(err_ptr != NULL){
				*err_ptr = body->err_num;
			}
			break;
		}while(1);
	}varfuture_concurrent_trigger_release(trigger);
	if(err != 0){
		errno = err;
	}
	return err ? -1:0;
}
/**
 *	\fn			void_future_timedget
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		[in] (void_future_t *)	future		:futureオブジェクト
 *	\param		[out](void*)			ret			:何もしない、示しがつかないので置いてるだけ
 *	\param		[out](int*)				err_ptr		:「処理内で」起こったerrnoの格納
 *	\param		[in] (int64_t)			tout_millis	:タイムアウト（ミリ秒）
 *	\return	(int)		:0で正常終了。0以外で待機時エラーとし、下記詳細をerrnoに格納する。
 *	\error		EINTR		:割り込み
 *				ECANCELED	:キャンセルされた
 *				ETIMEDOUT	:タイムアウト
 */
int		void_future_timedget(void_future_t *future, void* ret_ptr, int *err_ptr, int64_t tout_millis){
	varfuture_body_t		*body		= &(future->body);
	varfuture_trigger_t		*trigger	= body->trigger;
	int err = 0;
	//type ret = type_initial;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			if(body->state == varfuture_state_idle){
				/*idleの場合にはそのまま終わる。*/
			}
			else if(body->state == varfuture_state_abrt){
				/*abrtの場合、errnoを付ける*/
				err = ECANCELED;
			}
			else{
				/*waitやexecの場合はそのまま待機*/
				if(varfuture_concurrent_trigger_timedwait(trigger, tout_millis) < 0){
					err = errno;
					if(err == ETIMEDOUT){
						break;
					}
				}
				continue;
			}
			break;
		}while(1);
		
		if(ret_ptr != NULL){
			/* *ret_ptr = future->ret_val;*/
		}
		if(err_ptr != NULL){
			*err_ptr = body->err_num;
		}
	}varfuture_concurrent_trigger_release(trigger);
	
	if(err != 0){
		errno = err;
	}
	return err ? -1:0;
}

/**
 *	\fn			void_future_get
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 *	\remarks	idleの時に復帰するような動作。
 */
void	void_future_get_return(void_future_t *future){
	void_future_get(future, NULL, NULL);
}

/**
 *	\fn			void_future_timedget
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		(void_future_t *) 		future		:futureオブジェクト
 *	\param		(int64_t)				tout_millis	:タイムアウト（ミリ秒）
 *	\return	なし
 *	\remarks	tout_millis < 1でtout_millis = 0と等価
 */
void	void_future_timedget_return(void_future_t *future, int64_t tout_millis){
	void_future_timedget(future, NULL, NULL, tout_millis);
}

/**
 *	\fn			void_future_get_errno
 *	\brief		関数で起こったerrnoを参照する。終了してない場合は待機。
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 */
int		void_future_get_errno(void_future_t *future){
	int err_num = 0;
	printf("%s[%p]: ->errno is %d\n", __func__, future, future->body.err_num);
	void_future_get(future, NULL, &err_num);
	printf("%s[%p]: %d\n", __func__, future, err_num);
	return err_num;
}

/**
 *	\fn			void_future_timedget_errno
 *	\brief		関数で起こったerrnoを参照する。終了してない場合は待機。
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 */
int		void_future_timedget_errno(void_future_t *future, int64_t tout_millis){
	int err_num = 0;
	void_future_timedget(future, NULL, &err_num, tout_millis);
	return err_num;
}

/**
 *	\fn			void_future_redo
 *	\brief		パラメータやpromiseの情報はそのままに、もう一度待機する
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	再開できたら0。それ以外でマイナス
 */
int		void_future_redo(void_future_t *future){
	varfuture_trigger_t	*trigger	= future->body.trigger;
	varfuture_queue_t	*q			= future->body.queue;
	
	int ret = 0;
	varfuture_concurrent_trigger_acquire(trigger);{
		do{
			//ステート：idleとかabrtじゃないとダメ。
			switch(future->body.state){
			case varfuture_state_idle:
			case varfuture_state_abrt:
				break;
			default:
				ret = -1;
				break;
			}
			if(ret){
				break;
			}
			
			//モードごとに定めた条件
			switch(future->body.exec_mode){
			case varfuture_mode_isolate:
			case varfuture_mode_taskpool:
				//関数指定がないとダメ。
				if(future->task.fn_ptr == NULL){
					ret = -2;
				}
				break;
			default:
				break;
			}
			if(ret){
				break;
			}
			
			/*ステート遷移：待機*/
			future->body.state = varfuture_state_wait;
			
			/*モード設定が終わったので実行予約に投げ込んでおく*/
			switch(future->body.exec_mode){
			case varfuture_mode_taskpool:
			case varfuture_mode_promise:
				if((ret = varfuture_queue_append(q, &(future->body))) != 0){
					errno = EINVAL;
				}
				break;
			case varfuture_mode_isolate:
				if((ret = varfuture_concurrent_invoke_isolate(&(future->body))) != 0){
					/*スレッドの起動に失敗したらそのまま終わる。*/
					errno = ret;
					future->body.err_num = ret;
					future->body.state = varfuture_state_abrt;
				}
				break;
			default:
				break;
			}
		}while(0);
	}varfuture_concurrent_trigger_release(trigger);
	return ret;
}

/**
 *	\fn			void_promise_init
 *	\brief		promiseの初期化
 *	\param		(void_promise_t *)		promise	:promiseオブジェクト
 *	\return	int								:0で成功、それ以外で失敗。
 *	\remarks	グローバルからtriggerを借り受けて動作するイメージ。
 */
int		void_promise_init(void_promise_t *promise){
	varfuture_trigger_t	*future_trigger;
	varfuture_trigger_t	*modify_trigger;
	future_trigger = varfuture_concurrent_get_queue_future_trigger();
	modify_trigger = varfuture_concurrent_get_queue_modify_trigger();
	return varfuture_queue_init(&(promise->queue), future_trigger, modify_trigger);
}

/**
 *	\fn			void_promise_set
 *	\brief		promiseに関連付けたfutureに対して通知を行う
 *	\param		(void_promise_t *) 		promise	:promiseオブジェクト
 *	\param		(void*)					ret		:void_futureなので本来不要。示しがつかないので
 *	\param		int						err_num	:errno相当のもの。
 *	\return	通知できたfutureの数
 *	\remarks	ぶら下がったfutureに対してステータスのidle化とかを行う
 */
int		void_promise_set(void_promise_t *promise, void *ret, int err_num){
	varfuture_trigger_t	*trigger = varfuture_queue_get_trigger(&(promise->queue));
	void_future_t		*future = NULL;
	int ret_num = 0;
	for(;;){
		varfuture_concurrent_trigger_acquire(trigger);{
			future = (void_future_t *)varfuture_queue_get_task(&(promise->queue));
			if(future == NULL){
				varfuture_concurrent_trigger_release(trigger);
				break;
			}
			
			future->body.err_num = err_num;
			future->body.state = varfuture_state_idle;
			/*future->ret = ret;*/
			varfuture_concurrent_trigger_broadcast(future->body.trigger);
			ret_num++;
		}varfuture_concurrent_trigger_release(trigger);
	}
	return ret_num;
}
