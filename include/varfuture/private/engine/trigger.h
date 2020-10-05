/**
 *	\file		private/engine/trigger.h
 *	\brief		varfutureの並列処理系のうち、ロック/シグナル系統をまとめる
 *	\remarks	ラッパー関数的な感じでやりたいんだけど、いかんせんノウハウがないなあ
 */
#ifndef	PRIVATE_VARFUTURE_ENGINE_TRIGGER_H
#define	PRIVATE_VARFUTURE_ENGINE_TRIGGER_H

#include "../../varfuture_depend.h"

#ifdef	VARFUTURE_USE_PTHREAD
//pthread依存であるとわかったらpthreadで効くようインクルードする
#ifdef	NO_CONDATTR_SETLOCK
#include "./trigger_pthread.realtime.h"
#else
#include "./trigger_pthread.h"
#endif
#endif

/**
 *	\struct	var_future_trigger_s
 *	\brief		future.getから処理を呼び戻す際のトリガー
 */
struct varfuture_trigger_s{
	varfuture_lock_t	*lock;		//いわゆるpthread_mutex_t
	varfuture_cond_t	*cond;		//いわゆるpthread_cond_t
};


#endif	/* !PRIVATE_VARFUTURE_ENGINE_TRIGGER_H */
