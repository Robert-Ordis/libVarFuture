/**
 *	\file	type_future_prototypes.h
 *	\brief	型ごとのfutureオブジェクトのプロトタイプ宣言部分
 *	\brief	各関数のプロトタイプとベースの構造体を宣言する
 *
 */

#ifndef		TYPE_FUTURE_PROTOTYPES_H
#define		TYPE_FUTURE_PROTOTYPES_H

#include "./private/body.h"
#include "./private/pertype_functions_definition.h"
#include "./private/queue.h"
#include <stdint.h>

/**
 *	\def		varfuture_primitive_prototypes
 *	\brief		Witchcraft used in the header(*.h).
 *				Implement the types and function named "[type]_future_xxx".
 *	\param		ret_type		Return type, and basename.
 *	\remarks	NOT USABLE FOR POINTER OR STRUCTURE.
 *	\remarks	void is already implemented.
 */
#define	varfuture_primitive_prototypes(ret_type)\
	varfuture_define_prototypes(ret_type, ret_type)

/**
 *	\def		varfuture_define_prototypes
 *	\param		ret_type		futureオブジェクトが取り扱う返値の型。ポインタも構造体も一応行ける。
 *	\param		basename		各オブジェクト/関数の先頭につける名前。int_future_tとかint_future_getみたいな。
 *	\brief		これをもって、戻り値を伴う関数群及び関連オブジェクトの型を宣言する。
 *	\remarks	プリミティブ型ならret_typeとbasenameは同じになる。それ以外（構造体とかポインタ）は工夫してね。
 */
#define	varfuture_define_prototypes(ret_type, basename)\
	\
	/*コールバックについてまとめとく*/\
	typedef	ret_type	(*VARFUTURE_FUNC1_CB_(basename))(void*);\
	typedef	ret_type	(*VARFUTURE_FUNC2_CB_(basename))(void*,	void*);\
	typedef	ret_type	(*VARFUTURE_FUNC3_CB_(basename))(void*,	void*,	void*);\
	typedef	ret_type	(*VARFUTURE_FUNCV_CB_(basename))(int,	void**);\
	\
	/**
	 *	\typedef	basename##_future_t
	 *	\brief		ret_type##を返すfutureオブジェクト
	 */\
	typedef struct {\
		struct	varfuture_body_s	body;	/*親クラス相当*/\
		union {\
			VARFUTURE_FUNC1_CB_(basename)	one;\
			VARFUTURE_FUNC2_CB_(basename)	two;\
			VARFUTURE_FUNC3_CB_(basename)	three;\
			VARFUTURE_FUNCV_CB_(basename)	vector;\
			void*							fn_ptr;\
		} task;\
		ret_type					ret_val;	/*関数の戻り値*/\
	} VARFUTURE_T_(basename);\
	\
	/**
	 *	\struct	basename##_promise_t
	 *	\brief		ret_typeを手動で返すためのpromise。
	 */\
	typedef struct {\
		struct varfuture_queue_s	queue;\
	} VARPROMISE_T_(basename);\
	\
	\
	\
	/*ここから、関数群の宣言*/\
	\
	/**
	 *	\fn			basename##_future_init_for_exec
	 *	\brief		exec用に初期化
	 *	\param		(basename##_future_t *)		future	:futureオブジェクト
	 *	\return		int									:0で成功、それ以外で失敗
	 *	\remarks	スレッドプール内で呼んだら強制的にmode_isolateとなる
	 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
	 */\
	int			VARFUTURE_INIT_FOR_EXEC_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/**
	 *	\fn			basename##_future_init_for_isolate
	 *	\brief		独立スレッド用に初期化
	 *	\param		(basename##_future_t *)		future	:futureオブジェクト
	 *	\return		int									:0で成功、それ以外で失敗
	 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
	 */\
	int			VARFUTURE_INIT_FOR_ISOLATE_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/**
	 *	\fn			basename##_future_register_to_promise
	 *	\brief		promise用に初期化
	 *	\param		(basename##_future_t *)		future	:futureオブジェクト
	 *	\param		(basename##_promise_t *)	promise	:promiseオブジェクト
	 *	\return		int									:0で成功、それ以外で失敗
	 *	\remarks	戻り値待機中のfutureに対してこの関数を使ってはならない
	 */\
	int			VARFUTURE_REGISTER_TO_PROMISE_(basename)(VARFUTURE_T_(basename) *future, VARPROMISE_T_(basename) *promise);\
	\
	/**
	 *	\fn			basename##_future_exec1
	 *	\brief		引数１個のコールバックを依頼する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\param		(basename##_future_func1_cb)	func	:実際に行わせたい動作
	 *	\param		(void*)							arg1	:任意引数
	 *	\return		int										:0で予約成功、マイナスで失敗
	 */\
	int			VARFUTURE_EXEC1_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC1_CB_(basename) func, void *arg1);\
	\
	/**
	 *	\fn			basename##_future_exec2
	 *	\brief		引数２個のコールバックを依頼する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\param		(basename##_future_func2_cb)	func	:実際に行わせたい動作
	 *	\param		(void*)							arg1	:任意引数
	 *	\param		(void*)							arg2	:任意引数
	 *	\return		int										:0で予約成功、マイナスで失敗
	 */\
	int			VARFUTURE_EXEC2_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC2_CB_(basename) func, void *arg1, void *arg2);\
	\
	/**
	 *	\fn			basename##_future_exec3
	 *	\brief		引数３個のコールバックを依頼する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\param		(basename##_future_func3_cb)	func	:実際に行わせたい動作
	 *	\param		(void*)							arg1	:任意引数
	 *	\param		(void*)							arg2	:任意引数
	 *	\param		(void*)							arg3	:任意引数
	 *	\return		int										:0で予約成功、マイナスで失敗
	 */\
	int			VARFUTURE_EXEC3_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC3_CB_(basename) func, void *arg1, void *arg2, void *arg3);\
	\
	/**
	 *	\fn			basename##_future_execv
	 *	\brief		任意個引数のコールバックを依頼する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\param		(basename##_future_funcv_cb)	func	:実際に行わせたい動作
	 *	\param		(int)							argc	:引数の個数
	 *	\param		(void**)						argv	:引数配列
	 *	\return		int										:0で予約成功、マイナスで失敗
	 */\
	int			VARFUTURE_EXECV_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNCV_CB_(basename) func, int argc, void *argv[]);\
	\
	/**
	 *	\fn			basename##_future_cancel
	 *	\brief		予約済みのコールバック/プロミスをキャンセルする
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\return		int										:0でキャンセル成功、マイナスで失敗
	 *	\brief		idle及びwait状態で成功。exec状態では失敗しますよ。
	 *	\brief		getで待機しているスレッドはたたき起こされます。
	 *	\brief		成功した場合、get_errnoはECANCELED に設定されます
	 *	\brief		promiseは入れても何も起こらなくなります
	 */\
	int			VARFUTURE_CANCEL_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/**
	 *	\fn			basename##_future_is_finished
	 *	\brief		futureが何でもいいから終了したかどうかを返す
	 *	\param		[in] (basename##\future_t *)	future	:futureオブジェクト
	 *	\return		(int)									:終了してるなら1、それ以外で0
	 *	\remarks	要するに、getで待たされるようでなければOKの感覚（ガバ）
	 */\
	 int		VARFUTURE_IS_FINISHED_(basename)(VARFUTURE_T_(basename) *future);\
	 \
	/**
	 *	\fn			basename##_future_get
	 *	\brief		futureが終わるまで待機。また、戻り値とerrnoをポインタに書き写す
	 *	\param		[in] (basename##_future_t *)	future	:futureオブジェクト
	 *	\param		[out](ret_type*)				ret		:戻り値ポインタ。NULLでは何もしない
	 *	\param		[out](int*)						err_ptr	:「処理内で」起こったerrnoの格納。NULLで何もしない
	 *	\return		(int)									:0で正常終了。それ以外で待機時エラーとし、下記詳細をerrnoに格納する。
	 *	\error		EINTR		:割り込み
	 *				ECANCELED	:キャンセルされた
	 */\
	int			VARFUTURE_GET_(basename)(VARFUTURE_T_(basename) *future, ret_type *ret_ptr, int *err_ptr);\
	\
	/**
	 *	\fn			basename##_future_timedget
	 *	\brief		futureが終わるまでタイムアウト付きで待機。また、戻り値とerrnoをポインタに書き写す
	 *	\param		[in] (basename##_future_t *)	future		:futureオブジェクト
	 *	\param		[out](ret_type*)				ret			:戻り値ポインタ。NULLでは何もしない
	 *	\param		[out](int*)						err_ptr		:「処理内で」起こったerrnoの格納。NULLで何もしない
	 *	\param		[in] (int64_t)					tout_millis	:タイムアウト（ミリ秒）
	 *	\return		(int64_t)									:0で正常終了。0以外で待機時エラーとし、下記詳細をerrnoに格納する。
	 *	\error		EINTR		:割り込み
	 *				ECANCELED	:キャンセルされた
	 *				ETIMEDOUT	:タイムアウト。
	 */\
	int			VARFUTURE_TIMEDGET_(basename)(VARFUTURE_T_(basename) *future, ret_type *ret_ptr, int *err_ptr, int64_t tout_millis);\
	\
	/**
	 *	\fn			basename##_future_get_return
	 *	\brief		futureが終わるまで待機し、処理結果か渡されたreturnを取得する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\return		##ret_type##							:futureが受け持った戻り値
	 *	\remarks	idleの時に復帰するような動作。
	 */\
	ret_type	VARFUTURE_GET_RETURN_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/**
	 *	\fn			basename##_future_timedget_return
	 *	\brief		futureが終わるまでタイムアウト付きで待機し、処理結果か渡されたreturnを取得する
	 *	\param		(basename##_future_t *) 		future		:futureオブジェクト
	 *	\param		(int64_t)						tout_millis	:タイムアウト（ミリ秒）
	 *	\return		##ret_type##								:futureが受け持った戻り値
	 *	\remarks	tout_millis < 1でtout_millis = 0と等価
	 */\
	ret_type	VARFUTURE_TIMEDGET_RETURN_(basename)(VARFUTURE_T_(basename) *future, int64_t tout_millis);\
	\
	/**
	 *	\fn			basename##_future_get_errno
	 *	\brief		futureが終わるまで待機し、処理中で起こったか渡されるかしたerrnoを取得する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\return		(int)									:futureが受け持ったエラー値
	 *	\remarks	idleの時に復帰するような動作。
	 */\
	\
	int			VARFUTURE_GET_ERRNO_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/**
	 *	\fn			basename##_future_timedget_errno
	 *	\brief		futureが終わるまでタイムアウト付きで待機し、処理中で起こったか渡されるかしたerrnoを取得する
	 *	\param		(basename##_future_t *) 		future	:futureオブジェクト
	 *	\return		(int)									:futureが受け持ったエラー値
	 *	\remarks	idleの時に復帰するような動作。
	 */\
	int			VARFUTURE_TIMEDGET_ERRNO_(basename)(VARFUTURE_T_(basename) *future, int64_t tout_millis);\
	\
	/**
	 *	\fn			##basename##_future_redo
	 *	\brief		パラメータやpromiseの情報はそのままに、もう一度待機する
	 *	\param		(basename##_future_t *)		future	:futureオブジェクト
	 *	\return		(int)								:再開できたら0。それ以外でマイナス
	 */\
	int			VARFUTURE_REDO_(basename)(VARFUTURE_T_(basename) *future);\
	\
	/*ここから先はpromiseに関する関数群*/\
	\
	/**
	 *	\fn			basename##_promise_init
	 *	\brief		promiseの初期化
	 *	\param		(basename##_promise_t *)	promise	:promiseオブジェクト
	 *	\return		(int)								:0で成功、それ以外で失敗。
	 *	\remarks	グローバルからtriggerを借り受けて動作するイメージ。
	 */\
	int			VARPROMISE_INIT_(basename)(VARPROMISE_T_(basename) *promise);\
	\
	/**
	 *	\fn			basename##_promise_set
	 *	\brief		promiseに関連付けたfutureに対して戻り値を設定し、通知を行う
	 *	\param		(basename##_promise_t *) 	promise	:promiseオブジェクト
	 *	\param		(ret_type)					ret		:関連したfutureに持たせる戻り値
	 *	\param		(int)						err_num	:errno相当のもの。
	 *	\return		(int)							:通知できたfutureの数
	 *	\remarks	ぶら下がったfutureに対してステータスのidle化とかを行う
	 */\
	int			VARPROMISE_SET_(basename)(VARPROMISE_T_(basename) *promise, ret_type ret, int err_num);\

#endif		/* !TYPE_FUTURE_PROTOTYPES_H */
