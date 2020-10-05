/**
 *	\file		private/body.h
 *	\brief		varfuture表部分のうち、ベースになる構造体についてをまとめる
 *	\remarks	structの生書きでやるとなかなか表記が長くなってしまうのが悩み。
 */

#ifndef	PRIVATE_VARFUTURE_BODY_H
#define	PRIVATE_VARFUTURE_BODY_H

#include "../varfuture_attr.h"
struct varfuture_trigger_s;
struct varfuture_queue_s;
typedef struct varfuture_trigger_s varfuture_trigger_t;
typedef struct varfuture_body_s varfuture_body_t;
#define VARFUTURE_BODY_INIT_MEMBERS { .err_num = 0, .state = varfuture_state_idle, .arg = { .s = { .first = NULL, .second = NULL, .third = NULL } }}

/**
 *	\union		var_future_arg_u
 *	\brief		引数のまとめ
 */
union var_future_arg_u{
	struct {
		void	*first;
		void	*second;
		void	*third;
	} s;
	struct {
		int		c;
		void	**v;
	} v;
};



/**
 *	\struct	varfuture_body_s
 *	\brief		<type>_future_tの素体部分。（例えばvoid_future_tならこれがそのまま使われる）
 */
struct varfuture_body_s {
	int		err_num;							///出てきた最後のerrnoの格納
	varfuture_state_t				state;		///futureの状態
	union var_future_arg_u			arg;		///future単体に定めた各引数
	
	varfuture_mode_t				exec_mode;	///基本的な動作モード
	int								arg_mode;	///引数モード
	void	*(*func)(void*);					///コールバックとは別のポリモーフィズム起点用関数。外のAPIから自動的に決まる。	
	
	//スレッドプール連携用
	struct varfuture_queue_s*		queue;		///所属スレッドプールなりpromiseなり。がもっているキュー
	struct varfuture_body_s*		next;		///線形リスト接続をするのです。
	struct varfuture_trigger_s*		trigger;	///futureにおけるgetのブロック及び解放用
	
	//この下に、ret_valueとか実際の関数（task）とかが別構造体で乗っかる。
};

#endif	/* !PRIVATE_VARFUTURE_BODY_H */
 
