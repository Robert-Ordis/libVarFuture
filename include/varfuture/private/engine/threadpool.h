/**
 *	\file		private/engine/threadpool.h
 *	\brief		varfutureの並列処理系のうち、スレッドプール系統についてをまとめる
 *	\remarks	structの生書きでやるとなかなか表記が長くなってしまうのが悩み。
 */
#ifndef	PRIVATE_VARFUTURE_ENGINE_THREADPOOL_H
#define	PRIVATE_VARFUTURE_ENGINE_THREADPOOL_H

#include "../body.h"
#include "../queue.h"
#include "./trigger.h"

/*

struct varfuture_threadpool_s{
	
	//並列処理動作管理系統
	varfuture_thread_t				th;			///実際に動くスレッド
	
	//スレッドプールとしてのデータ構造
	int								in_active;	///動いているかどうか。1で動作中
	struct varfuture_taskqueue_s	*queue;		///スレッドに持たせるタスクキュー
};


varfuture_threadpool_boot(struct varfuture_threadpool_s *tp, struct varfuture_taskqueue_s *queue);
*/

struct varfuture_threadpool_s{
	
	//並列処理動作管理系統
	varfuture_thread_t			th;					///実際に動くスレッド
	
	//スレッドプールとしてのデータ構造
	int							in_active;			///動いているかどうか。1で動作中
	struct varfuture_queue_s	*queue;				///自身が参照するべきタスクキュー
};

//スレッドプールのタスク。特にfuture部分に出す必要もないのでここで記述
int varfuture_threadpool_init(struct varfuture_threadpool_s *self, struct varfuture_queue_s *queue);
int varfuture_threadpool_boot(struct varfuture_threadpool_s *self);
int varfuture_threadpool_down(struct varfuture_threadpool_s *self);

#define varfuture_threadpool_is_current_thread(self) varfuture_thread_equal((self)->th, varfuture_thread_current())

#endif	/* !PRIVATE_VARFUTURE_ENGINE_THREADPOOL_H */
