/**
 *	\file		private/queue_api.h
 *	\brief		varfutureを取りまとめるタスクキューのAPI部分
 *	\remarks	できるだけ本筋は秘匿しておくのが筋ってもん
 */

#ifndef	PRIVATE_VARFUTURE_QUEUE_API_H
#define	PRIVATE_VARFUTURE_QUEUE_API_H


typedef struct varfuture_queue_s varfuture_queue_t;
struct varfuture_body_s;
struct varfuture_trigger_s;

//初期化。今後のために外から同期用のトリガーをもらっておく
int	varfuture_queue_init(varfuture_queue_t *queue, struct varfuture_trigger_s *future_trigger, struct varfuture_trigger_s *append_trigger);

//トリガーを貸し出す
struct varfuture_trigger_s*	varfuture_queue_get_trigger(varfuture_queue_t *queue);

//編集
int	varfuture_queue_append(varfuture_queue_t *queue, struct varfuture_body_s *body);
int varfuture_queue_remove(varfuture_queue_t *queue, struct varfuture_body_s *body);

//取得。ただし、待ったりはしない
struct varfuture_body_s*	varfuture_queue_get_task(varfuture_queue_t *queue);

//取得。それもちゃんと待機して行う。
struct varfuture_body_s*	varfuture_queue_get_task_with_wait(varfuture_queue_t *queue);


#endif	/* !PRIVATE_VARFUTURE_QUEUE_API_H */
 
