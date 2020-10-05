/**
 *	\file		engine_threadpool.c
 *	\brief		エンジン部分のうち、スレッドプール関連の実装についてまとめたもの
 *	
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./include/varfuture/private/engine/trigger.h"
#include "./include/varfuture/private/body.h"
#include "./include/varfuture/private/engine/threadpool.h"
#include "./include/varfuture/private/engine/concurrent_api.h"
#include "./include/varfuture/varfuture_depend.h"
#include "./include/varfuture/private/queue_api.h"

//スレッドプールの初期化。lockもret_condももう準備は済ませてる。１度しか呼ばない。
//クラスとしてはあまりに不安ではあるが、公開関数ではないのでまあ気にしないでいいだろう。多分
int varfuture_threadpool_init(struct varfuture_threadpool_s *self, struct varfuture_queue_s *queue){
	//int step = 0;
	//int error = 0;
	assert(queue != NULL);
	//errno = 0;
	//スレッドプール制御部の初期化
	self->in_active = 0;
	
	//スレッドプールデータ部の初期化
	//トリガーも全部queueがもっています
	self->queue = queue;
	
	return 0;
	
}

static void *varfuture_threadpool_task(void *arg){
	struct varfuture_threadpool_s	*self = (struct varfuture_threadpool_s *) arg;
	struct varfuture_body_s			*task = NULL;
	varfuture_trigger_t				*trigger = varfuture_queue_get_trigger(self->queue);
	int errnum;
	
	varfuture_trigger_acquire(trigger, &errnum);{
		//printf("%s[%p]: start task\n", __func__, self);
		while(self->in_active){
			//printf("%s[%p]: get task\n", __func__, self);
			if((task = varfuture_queue_get_task_with_wait(self->queue)) == NULL){
				continue;
			}
			
			//ここに来た時点で何かしらのタスクは存在することになる。
			varfuture_trigger_release(trigger, &errnum);
			
			task->func(task);
			
			varfuture_trigger_acquire(trigger, &errnum);
		}
	}varfuture_trigger_release(trigger, &errnum);
	//printf("%s[%p]: stop task\n", __func__, self);
	return NULL;
}

#ifdef	VARFUTURE_USE_PTHREAD
#include <pthread.h>
//スレッドプールの発進。
int varfuture_threadpool_boot(struct varfuture_threadpool_s *self) {
	int ret = 0;
	//int error = 0;
	do{
		if(self->in_active){
			ret = -1;
		break;
		}
		self->in_active = 1;
		//printf("%s: boot [%p]\n", __func__, self);
		if((ret = pthread_create(&self->th, NULL, varfuture_threadpool_task, self)) < 0){
			break;
		}
		pthread_detach(self->th);
	}while (0);
	return ret;
}

//スレッドプールのダウン
int varfuture_threadpool_down(struct varfuture_threadpool_s *self) {
	int ret = 0;
	//int error = 0;
	do{
		if(!self->in_active){
			ret = -1;
			break;
		}
		self->in_active = 0;
	}while (0);
	return ret;
}

#endif	/* VARFUTURE_USE_PTHREAD */

