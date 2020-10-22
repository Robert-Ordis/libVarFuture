/**
 *	\file		varfuture_queue.c
 *	\brief		futureエンジンパーツのうち、タスクキュー。
 *	
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "varfuture/private/body.h"
#include "varfuture/private/queue_api.h"
#include "varfuture/private/queue.h"
#include "varfuture/private/engine/trigger.h"


//初期化。今後のために外から同期用のトリガーをもらっておく
int	varfuture_queue_init(varfuture_queue_t *queue, struct varfuture_trigger_s *future_trigger, struct varfuture_trigger_s *modify_trigger){
	queue->watcher_count = 0;
	queue->head = NULL;
	queue->future_trigger = future_trigger;
	queue->modify_trigger = modify_trigger;
	queue->closed = 0;
	return 0;
}

//トリガーを貸し出す
struct varfuture_trigger_s*	varfuture_queue_get_trigger(varfuture_queue_t *queue){
	return queue->future_trigger;
}

//編集：追加
int	varfuture_queue_append(varfuture_queue_t *queue, struct varfuture_body_s *future){
	struct varfuture_body_s **insert;
	int errnum;
	for(insert = &queue->head; *(insert) != NULL; insert = &((*insert)->next)){
		//つながる箇所にたどり着くまで繰り返し
		//また、既にリスト内にいたらそれはNG。
		if(*insert == future){
			return -1;
		}
	}
	*insert = future;
	future->next = NULL;
	if(queue->watcher_count){
		varfuture_trigger_signal(queue->modify_trigger, &errnum);
	}
	return 0;
}

//編集：削除
int varfuture_queue_remove(varfuture_queue_t *queue, struct varfuture_body_s *future){
	struct varfuture_body_s *cursor;
	//int errnum;
	if(future == queue->head){
		queue->head = future->next;
	}
	else{
		for(cursor = queue->head; (cursor != NULL && cursor->next != future); cursor = cursor->next){
		}
		if(cursor == NULL){
			return -1;
		}
		//リンクをつなぎなおす。
		cursor->next = future->next;
	}
	return 0;
}

//取得。ただし、待ったりはしない
struct varfuture_body_s*	varfuture_queue_get_task(varfuture_queue_t *queue){
	struct varfuture_body_s *ret = queue->head;
	if(ret != NULL){
		queue->head = ret->next;
	}
	return ret;
}

//取得。それもちゃんと待機して行う。
struct varfuture_body_s*	varfuture_queue_get_task_with_wait(varfuture_queue_t *queue){
	struct varfuture_body_s *ret = NULL;
	int errnum;
	queue->watcher_count ++;
	for(;;){
		errnum = 0;
		if(!queue->closed && (ret = varfuture_queue_get_task(queue)) == NULL){
			varfuture_trigger_wait(queue->modify_trigger, &errnum);
			if(errnum != 0){
				errno = errnum;
				break;
			}
			continue;
		}
		break;
	}
	queue->watcher_count --;
	return ret;
}

//締め出し
int varfuture_queue_lockout_waiter(varfuture_queue_t *queue){
	queue->closed = 1;
	int errnum = 0;
	varfuture_trigger_broadcast(queue->modify_trigger, &errnum);
	if(errnum != 0){
		errno = errnum;
		return -1;
	}
	return 0;
}
