/**
 *	\file		voidfuture.h
 *	\brief		void型のfutureに着目したもの。これがベースとなるイメジ。
 */
#ifndef	VOID_FUTURE_H
#define	VOID_FUTURE_H

#include "./private/body.h"
#include "./varfuture_attr.h"
#include "./private/queue.h"
#include <stdint.h>

///引数１のコールバック
typedef	void (*void_future_func1_cb)(void*);
///引数２のコールバック
typedef	void (*void_future_func2_cb)(void*, void*);
///引数３のコールバック
typedef	void (*void_future_func3_cb)(void*, void*, void*);
///任意個引数のコールバック
typedef	void (*void_future_funcv_cb)(int, void**);

union void_future_fn_u {
	void_future_func1_cb	one;
	void_future_func2_cb	two;
	void_future_func3_cb	three;
	void_future_funcv_cb	vector;
	void*					fn_ptr;
};

/**
 *	\struct	void_future_t
 *	\brief		リターン無しのfuture。試金石として作成。
 *	\remarks	errno相当のものは返せますよ。
 */
typedef struct {
	struct	varfuture_body_s	body;
	union	void_future_fn_u	task;
} void_future_t;

/**
 *	\struct	void_promise_t
 *	\brief		リターン無しのpromise。試金石として作成。
 *	\remarks	errno相当のものは返せますよ。
 */
typedef struct {
	struct	varfuture_queue_s	queue;
} void_promise_t;

/**
 *	\fn			void_future_init_for_exec
 *	\brief		exec用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	スレッドプール内で呼んだら強制的にmode_isolateとなる
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_init_for_exec(void_future_t *future);

/**
 *	\fn			void_future_init_for_isolate
 *	\brief		独立スレッド用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_init_for_isolate(void_future_t *future);
	
/**
 *	\fn			void_future_register_to_promise
 *	\brief		promise用に初期化
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\param		(void_promise_t *)		promise	:promiseオブジェクト
 *	\return	int								:0で成功、それ以外で失敗
 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
 */
int		void_future_register_to_promise(void_future_t *future, void_promise_t *promise);
	
/**
 *	\fn			void_future_exec1
 *	\brief		引数１個のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_func1_cb)	func	:実際に行わせたい動作
 *	\param		(void*)					arg1	:任意引数
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_exec1(void_future_t *future, void_future_func1_cb func, void *arg1);

/**
 *	\fn			void_future_exec2
 *	\brief		引数２個のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_func2_cb)	func	:実際に行わせたい動作
 *	\param		(void*)					arg1	:任意引数
 *	\param		(void*)					arg2	:任意引数
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_exec2(void_future_t *future, void_future_func2_cb func, void *arg1, void *arg2);

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
int		void_future_exec3(void_future_t *future, void_future_func3_cb func, void *arg1, void *arg2, void *arg3);

/**
 *	\fn			void_future_execv
 *	\brief		任意個引数のコールバックを依頼する
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\param		(void_future_funcv_cb)	func	:実際に行わせたい動作
 *	\param		(int)					argc	:引数の個数
 *	\param		(void**)				argv	:引数配列
 *	\return	0で予約成功、マイナスで失敗
 */
int		void_future_execv(void_future_t *future, void_future_funcv_cb func, int argc, void *argv[]);


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
int		void_future_cancel(void_future_t *future);

/**
 *	\fn			void_future_is_finished
 *	\brief		futureが何でもいいから終了したかどうかを返す
 *	\param		[in] (void\future_t *)	future	:futureオブジェクト
 *	\return	(int)							:終了してるなら1、それ以外で0
 */
int		void_future_is_finished(void_future_t *future);


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
int		void_future_get(void_future_t *future, void* ret_ptr, int *err_ptr);

/**
 *	\fn			void_future_timedget
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		[in] (void_future_t *)	future		:futureオブジェクト
 *	\param		[out](void*)			ret			:何もしない、示しがつかないので置いてるだけ
 *	\param		[out](int*)				err_ptr		:「処理内で」起こったerrnoの格納
 *	\param		[in] (int64_t)			tout_millis	:タイムアウト（ミリ秒）
 *	\return	(int64_t)		:0で正常終了。0以外で待機時エラーとし、下記詳細をerrnoに格納する。
 *	\error		EINTR		:割り込み
 *				ECANCELED	:キャンセルされた
 *				ETIMEDOUT	:タイムアウト。この場合は戻り値が正の数となり、残りミリ秒が返される
 */
int		void_future_timedget(void_future_t *future, void* ret_ptr, int *err_ptr, int64_t tout_millis);

/**
 *	\fn			void_future_get
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 *	\remarks	idleの時に復帰するような動作。
 */
void	void_future_get_return(void_future_t *future);

/**
 *	\fn			void_future_timedget
 *	\brief		（void型用なので）処理が終わるまでの待機
 *	\param		(void_future_t *) 		future		:futureオブジェクト
 *	\param		(int64_t)				tout_millis	:タイムアウト（ミリ秒）
 *	\return	なし
 *	\remarks	tout_millis < 1でtout_millis = 0と等価
 */
void	void_future_timedget_return(void_future_t *future, int64_t tout_millis);

/**
 *	\fn			void_future_get_errno
 *	\brief		関数で起こったerrnoを参照する。終了してない場合は待機。
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 */
int		void_future_get_errno(void_future_t *future);

/**
 *	\fn			void_future_timedget_errno
 *	\brief		関数で起こったerrnoを参照する。終了してない場合は待機。
 *	\param		(void_future_t *) 		future	:futureオブジェクト
 *	\return	なし
 */
int		void_future_timedget_errno(void_future_t *future, int64_t tout_millis);

/**
 *	\fn			void_future_redo
 *	\brief		パラメータやpromiseの情報はそのままに、もう一度待機する
 *	\param		(void_future_t *)		future	:futureオブジェクト
 *	\return	再開できたら0。それ以外でマイナス
 */
int		void_future_redo(void_future_t *future);

/**
 *	\fn			void_promise_init
 *	\brief		promiseの初期化
 *	\param		(void_promise_t *)		promise	:promiseオブジェクト
 *	\return	int								:0で成功、それ以外で失敗。
 *	\remarks	グローバルからtriggerを借り受けて動作するイメージ。
 */
int		void_promise_init(void_promise_t *promise);

/**
 *	\fn			void_promise_set
 *	\brief		promiseに関連付けたfutureに対して通知を行う
 *	\param		(void_promise_t *) 		promise	:promiseオブジェクト
 *	\param		(void *)				ret		:何もしない。示しがつかないので付けただけ
 *	\param		int						err_num	:errno相当のもの。
 *	\return	通知できたfutureの数
 *	\remarks	ぶら下がったfutureに対してステータスのidle化とかを行う
 */
int		void_promise_set(void_promise_t *promise, void *ret, int err_num);

#endif	/* !VOID_FUTURE_H */
