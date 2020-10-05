/**
 *	\file		private/engine/concurrent_api.h
 *	\brief		varfutureの並列処理系にかかわる関数をまとめる
 *	\remarks	structの生書きでやるとなかなか表記が長くなってしまうのが悩み。
 *	\remarks	スレッドプール構造を隠す関係上、ロック云々も付けているが<type>_future_tのtriggerのロックと等価
 */
#ifndef	PRIVATE_VARFUTURE_ENGINE_CONCURRENT_API_H
#define	PRIVATE_VARFUTURE_ENGINE_CONCURRENT_API_H

#include <stdint.h>

#include "../body.h"
#include "../queue_api.h"

typedef	struct varfuture_threadpool_s	varfuture_threadpool_t;

/**
 *	\fn			varfuture_concurrent_init
 *	\brief		初期化。それに伴ってバックグラウンドスレッドの起動
 *	\param		(int)tp_num		:スレッド数
 *	\return	(int)起動できた数。-1でmalloc/-2でスレッド起動エラー。かつerrno格納。
 *	\remarks	tp_numは1以下の場合では強制的にデフォルト数起動とする
 */
int	varfuture_concurrent_init(int tp_num);

/**
 *	\fn			varfuture_concurrent_get_threadpool_queue
 *	\brief		スレッドプール処理用のタスクキューを取得する
 *	\return	スレッドプール処理用のタスクキュー
 *	\remarks	varfuture_concurrent_initを出したことを前提とします
 *	\remarks	
 */
varfuture_queue_t*	varfuture_concurrent_get_threadpool_queue();

/**
 *	\fn			varfuture_concurrent_get_isolate_trigger
 *	\brief		スレッドプールとは別の独立トリガーをもらい受ける
 *	\return	(ptr)独立動作用のトリガー。malloc無しで動かす都合上多分全部一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_isolate_trigger();

/**
 *	\fn			varfuture_concurrent_get_queue_modify_trigger
 *	\brief		タスクキュー処理用のプール処理トリガーをもらう
 *	\return	(ptr)グローバル用のトリガー。malloc無しで動かす都合上多分ロックは一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_queue_modify_trigger();

/**
 *	\fn			varfuture_concurrent_get_queue_future_trigger
 *	\brief		タスクキュー処理用のfuture通知トリガーをもらう
 *	\return	(ptr)グローバル用のトリガー。malloc無しで動かす都合上多分ロックは一緒
 */
struct varfuture_trigger_s* varfuture_concurrent_get_queue_future_trigger();

/**
 *	\fn			varfuture_concurrent_invoke_isolate
 *	\brief		独立future用に１つ起動する
 *	\return	(int)0で成功。負値で失敗（errnoにそれっぽいこと書く）
 */
int	varfuture_concurrent_invoke_isolate(struct varfuture_body_s *future);

/**
 *	\fn			varfuture_concurrent_trigger_acquire
 *	\brief		指定のトリガーをもってロックを獲得する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_acquire(struct varfuture_trigger_s *trigger);

/**
 *	\fn			varfuture_concurrent_trigger_release
 *	\brief		指定のトリガーをもってロックを解放する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_release(struct varfuture_trigger_s *trigger);

/**
 *	\fn			varfuture_concurrent_trigger_wait
 *	\brief		指定のトリガーをもって信号を待機する
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_wait(struct varfuture_trigger_s *trigger);

/**
 *	\fn			varfuture_concurrent_trigger_timedwait
 *	\brief		指定のトリガーをもって信号を待機する
 *	\param		(ptr)trigger			:トリガー。全部とまでは言わないけど大体同じ
 *	\param		(int64_t)tout_millis	:街時刻
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_timedwait(struct varfuture_trigger_s *trigger, int64_t tout_millis);

/**
 *	\fn			varfuture_concurrent_trigger_signal
 *	\brief		指定のトリガーをもって待機している人誰か一人起こす
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_signal(struct varfuture_trigger_s *trigger);

/**
 *	\fn			varfuture_concurrent_trigger_notify
 *	\brief		指定のトリガー（の条件変数）で待機してる子全員起こす
 *	\param		(ptr)	:トリガー。全部とまでは言わないけど大体同じ
 *	\return	(int)成功時0。失敗時は負の値かつerrnoに何かつく
 */
int	varfuture_concurrent_trigger_broadcast(struct varfuture_trigger_s *trigger);


#endif	/* !PRIVATE_VARFUTURE_ENGINE_CONCURRENT_API_H */
