/**
 *	\file	varfuture_attr.h
 *	\brief	varFutureライブラリの固定値を押し込める
 */
#ifndef	VARFUTURE_ATTR_H
#define	VARFUTURE_ATTR_H

/**
 *	\enum		var_future_mode_t
 *	\brief		どのようなモードで動かすかを定義する
 */
typedef enum {
	varfuture_mode_taskpool,	///前もって用意したスレッドプールで実行する
	varfuture_mode_isolate,	///futureオブジェクト一つに割り当てたスレッドで実行する
	varfuture_mode_promise		///promiseオブジェクトと連動する
} varfuture_mode_t;

/**
 *	\enum		var_future_state_t
 *	\brief		futureのステータスを現す
 *	\remarks	例えばfuture_execNを出したりpromise_bindしたりしたらもれなくproc状態になる
 */
typedef enum {
	varfuture_state_idle = 0,	///初期化時。もしくは終了後。getしたらすぐ帰る。
	varfuture_state_wait,		///処理待ち。cancelすることは一応できる。
	varfuture_state_exec,		///まさしく処理中。cancelできない。
	varfuture_state_abrt		///結果をもらう前に中断命令をもらった場合
} varfuture_state_t;

#endif	/* !VARFUTURE_ATTR_H */
 