/**
 *	\file		private/queue.h
 *	\brief		varfutureを取りまとめるタスクキューの構造体部分
 *	\remarks	できるだけ本筋は秘匿しておくのが筋ってもん
 */

#ifndef	PRIVATE_VARFUTURE_QUEUE_H
#define	PRIVATE_VARFUTURE_QUEUE_H

struct varfuture_trigger_s;
struct varfuture_body_s;

/**
 *	\struct	varfuture_queue_s
 *	\brief		<type>_future_tをため込んでおくパーツ。
 *	\remarks	threadpool及びpromiseの２つに用いる予定
 */
struct varfuture_queue_s {
	int		watcher_count;							///見ている人のカウント。1以上ならmodify_triggerによるsignalを行う
	int		closed;									///1でキューがひとまず終わらなきゃって状態になってる
	
	struct	varfuture_body_s	*head;				///頭。大体これを取って実行する
	struct	varfuture_trigger_s	*future_trigger;	///所属してくれるfutureに貸し出すtrigger。自分のこれもまた借り物。
	struct	varfuture_trigger_s	*modify_trigger;	///自身が編集された際にたたくtrigger。future_triggerとlockは同じである
	
	
};

#endif	/* !PRIVATE_VARFUTURE_QUEUE_H */
 
