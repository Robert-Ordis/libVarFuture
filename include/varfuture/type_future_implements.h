/**
 *	\file	type_future_implements.h
 *	\brief	型ごとのfutureオブジェクトにかかわる関数の実装
 */
#ifndef		TYPE_FUTURE_IMPLEMENTS_H
#define		TYPE_FUTURE_IMPLEMENTS_H

#include "./private/pertype_functions_definition.h"
#include "./private/inline_processes.h"


/**
 *	\def		varfuture_primitive_implements
 *	\brief		Witchcraft used in the code(*.c).
 *				Implement the function named "[type]_future_xxx".
 *	\param		ret_type		Return type, and basename. NOT USABLE FOR POINTER OR STRUCTURE.
 *	\param		ret_init		initial value.
 */
#define	varfuture_primitive_implements(ret_type, ret_init)\
	varfuture_define_implements(ret_type, ret_type, ret_init)

/**
 *	\def		varfuture_define_implements
 *	\param		ret_type		futureオブジェクトが取り扱う返値の型。ポインタも構造体も一応行ける。
 *	\param		basename		各オブジェクト/関数の先頭につける名前。int_future_tとかint_future_getみたいなものができる
 *	\param		ret_init		戻り値の初期値
 *	\brief		これをもって、戻り値を伴う関数群を実装する。
 *	\remarks	プリミティブ型ならret_typeとbasenameは同じになる。それ以外（構造体とかポインタ）は工夫してね。
 */
#define	varfuture_define_implements(ret_type, basename, ret_init)\
	/*futureタスクが行う関数の起点*/\
	static void	*VARFUTURE_TASK_(basename)(void *arg){\
		VARFUTURE_T_(basename) *future = (VARFUTURE_T_(basename) *)arg;\
		varfuture_body_t	*body = &(future->body);\
		varfuture_trigger_t	*trigger = body->trigger;\
		int err_num = 0;\
		\
		/*ステータス遷移：exec（＝これをもってキャンセルできなくなる）*/\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*何かを間違えてスレッド実行中にキャンセルが入ってもカバーしましょう*/\
				if(body->state != varfuture_state_wait){\
					err_num = ECANCELED;\
					body->err_num = ECANCELED;\
					break;\
				}\
				body->state = varfuture_state_exec;\
			} while (0);\
		}varfuture_concurrent_trigger_release(trigger);\
		if(err_num != 0){\
			return NULL;\
		}\
		errno = 0;\
		/*実行モードによって関数の振り分け方を変える。*/\
		switch(body->arg_mode){\
		case 1:/*引数１*/\
			future->ret_val = future->task.one(body->arg.s.first);\
			break;\
		case 2:/*引数１*/\
			future->ret_val = future->task.two(body->arg.s.first, body->arg.s.second);\
			break;\
		case 3:/*引数１*/\
			future->ret_val = future->task.three(body->arg.s.first, body->arg.s.second, body->arg.s.third);\
			break;\
		default:/*任意個引数*/\
			future->ret_val = future->task.vector(body->arg.v.c, body->arg.v.v);\
			break;\
		}\
		err_num = errno;\
		\
		/*ステータス遷移。idleへ移行（＝もう自由）*/\
		varfuture_concurrent_trigger_acquire(trigger);{\
			body->state = varfuture_state_idle;\
			body->err_num = err_num;\
			varfuture_concurrent_trigger_broadcast(trigger);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return NULL;\
	}\
	/*初期化：スレッドプール起動用*/\
	int			VARFUTURE_INIT_FOR_EXEC_(basename)(VARFUTURE_T_(basename) *future){\
		static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;\
		static ret_type			r = ret_init;\
		varfuture_queue_t *q = varfuture_concurrent_get_threadpool_queue();\
		if(q == NULL){\
			return VARFUTURE_INIT_FOR_ISOLATE_(basename)(future);\
		}\
		future->body = b;\
		future->body.exec_mode = varfuture_mode_taskpool;\
		future->body.queue = q;\
		future->body.trigger = varfuture_queue_get_trigger(q);\
		future->ret_val = r;\
		return 0;\
	}\
	\
	/*初期化：独立スレッド用*/\
	int			VARFUTURE_INIT_FOR_ISOLATE_(basename)(VARFUTURE_T_(basename) *future){\
		static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;\
		static ret_type			r = ret_init;\
		\
		future->body = b;\
		future->body.exec_mode = varfuture_mode_isolate;\
		future->body.queue = NULL;\
		future->body.trigger = varfuture_concurrent_get_isolate_trigger();\
		future->ret_val = r;\
		return 0;\
	}\
	\
	/*初期化：プロミス用*/\
	int			VARFUTURE_REGISTER_TO_PROMISE_(basename)(VARFUTURE_T_(basename) *future, VARPROMISE_T_(basename) *promise){\
		static varfuture_body_t b = VARFUTURE_BODY_INIT_MEMBERS;\
		static ret_type			r = ret_init;\
		\
		varfuture_trigger_t	*trigger = varfuture_queue_get_trigger(&(promise->queue));\
		int ret = 0;\
		\
		varfuture_concurrent_trigger_acquire(trigger);{\
			future->body = b;\
			future->body.exec_mode = varfuture_mode_promise;\
			future->body.queue = &(promise->queue);\
			future->ret_val = r;\
			\
			if((ret = varfuture_queue_append(&(promise->queue), &(future->body))) == 0){\
				future->body.trigger = trigger;\
				future->body.state = varfuture_state_wait;\
			}\
			\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*処理起動：引数１*/\
	int			VARFUTURE_EXEC1_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC1_CB_(basename) func, void *arg1){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*起動前検査*/\
				VARFUTURE_BODY_BEFORE_EXEC_(body, ret);\
				if(ret != 0){\
					errno = EINVAL;\
					break;\
				}\
				\
				/*OKなので実行しましょうか*/\
				\
				/*ステート遷移：待機*/\
				body->state = varfuture_state_wait;\
				\
				/*モード設定：引数１モードでいろいろ設定*/\
				future->task.one = func;\
				body->arg_mode = 1;\
				body->arg.s.first = arg1;\
				body->func = VARFUTURE_TASK_(basename);\
				\
				/*起動*/\
				VARFUTURE_BODY_EXEC_INVOKE_(body, ret);\
				\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*処理起動：引数２*/\
	int			VARFUTURE_EXEC2_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC2_CB_(basename) func, void *arg1, void *arg2){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*起動前検査*/\
				VARFUTURE_BODY_BEFORE_EXEC_(body, ret);\
				if(ret != 0){\
					errno = EINVAL;\
					break;\
				}\
				\
				/*OKなので実行しましょうか*/\
				\
				/*ステート遷移：待機*/\
				body->state = varfuture_state_wait;\
				\
				/*モード設定：引数２モードでいろいろ設定*/\
				future->task.two = func;\
				body->arg_mode = 2;\
				body->arg.s.first = arg1;\
				body->arg.s.second = arg2;\
				body->func = VARFUTURE_TASK_(basename);\
				\
				/*起動*/\
				VARFUTURE_BODY_EXEC_INVOKE_(body, ret);\
				\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*処理起動：引数３*/\
	int			VARFUTURE_EXEC3_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNC3_CB_(basename) func, void *arg1, void *arg2, void *arg3){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*起動前検査*/\
				VARFUTURE_BODY_BEFORE_EXEC_(body, ret);\
				if(ret != 0){\
					errno = EINVAL;\
					break;\
				}\
				\
				/*OKなので実行しましょうか*/\
				\
				/*ステート遷移：待機*/\
				body->state = varfuture_state_wait;\
				\
				/*モード設定：引数３モードでいろいろ設定*/\
				future->task.three = func;\
				body->arg_mode = 3;\
				body->arg.s.first = arg1;\
				body->arg.s.second = arg2;\
				body->arg.s.third = arg3;\
				body->func = VARFUTURE_TASK_(basename);\
				\
				/*起動*/\
				VARFUTURE_BODY_EXEC_INVOKE_(body, ret);\
				\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*処理起動：任意個引数*/\
	int			VARFUTURE_EXECV_(basename)(VARFUTURE_T_(basename) *future, VARFUTURE_FUNCV_CB_(basename) func, int argc, void *argv[]){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*起動前検査*/\
				VARFUTURE_BODY_BEFORE_EXEC_(body, ret);\
				if(ret != 0){\
					errno = EINVAL;\
					break;\
				}\
				\
				/*OKなので実行しましょうか*/\
				\
				/*ステート遷移：待機*/\
				body->state = varfuture_state_wait;\
				\
				/*モード設定：argvモードでいろいろ設定*/\
				future->task.vector = func;\
				body->arg_mode = -1;\
				body->arg.v.c = argc;\
				body->arg.v.v = argv;\
				body->func = VARFUTURE_TASK_(basename);\
				\
				/*起動*/\
				VARFUTURE_BODY_EXEC_INVOKE_(body, ret);\
				\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*futureのキャンセル*/\
	int			VARFUTURE_CANCEL_(basename)(VARFUTURE_T_(basename) *future){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		varfuture_queue_t		*q;\
		\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*動作中の状態で設定しようとしても駄目です*/\
				switch(body->state){\
				case varfuture_state_idle:\
				case varfuture_state_abrt:\
					ret = -1;\
					break;\
				case varfuture_state_exec:\
					ret = -2;\
					break;\
				default:\
					break;\
				}\
				\
				if(ret < 0){\
					break;\
				}\
				\
				\
				/*各モードに見合ったキャンセル操作を行う*/\
				/*ステート遷移：中断*/\
				body->state = varfuture_state_abrt;\
				body->err_num = ECANCELED;\
				switch(body->exec_mode){\
				case varfuture_mode_isolate:\
					/*どこかに属してるわけでもないので特にやることない、と思う*/\
					break;\
				case varfuture_mode_taskpool:\
				case varfuture_mode_promise:\
					/*スレッドプールやpromiseに存在するキューから除名*/\
					q = body->queue;\
					varfuture_queue_remove(q, body);\
					break;\
				default:\
					ret = -2;\
					break;\
				}\
				varfuture_concurrent_trigger_broadcast(trigger);\
				\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*確認：futureが終わったのかどうか*/\
	int			VARFUTURE_IS_FINISHED_(basename)(VARFUTURE_T_(basename) *future){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int ret = 0;\
		\
		varfuture_concurrent_trigger_acquire(trigger);{\
			if(body->state != varfuture_state_exec && body->state != varfuture_state_wait){\
				ret = 1;\
			}\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		return ret;\
	}\
	\
	/*待機：精密な結果の取得*/\
	int			VARFUTURE_GET_(basename)(VARFUTURE_T_(basename) *future, ret_type *ret_ptr, int *err_ptr){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int err = 0;\
		\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				if(body->state == varfuture_state_idle){\
					/*idleの場合にはそのまま終わる。*/\
				}\
				else if(body->state == varfuture_state_abrt){\
					/*abrtの場合、errnoを付ける*/\
					err = ECANCELED;\
				}\
				else{\
					/*waitやexecの場合はそのまま待機*/\
					if(varfuture_concurrent_trigger_wait(trigger) < 0){\
						err = errno;\
					}\
					continue;\
				}\
				if(ret_ptr != NULL){\
					*ret_ptr = future->ret_val;\
				}\
				if(err_ptr != NULL){\
					*err_ptr = body->err_num;\
				}\
				break;\
			}while(1);\
		}varfuture_concurrent_trigger_release(trigger);\
		if(err != 0){\
			errno = err;\
		}\
		return err ? -1:0;\
	}\
	\
	/*待機：精密な結果の取得/タイムアウト（ミリ秒）*/\
	int			VARFUTURE_TIMEDGET_(basename)(VARFUTURE_T_(basename) *future, ret_type *ret_ptr, int *err_ptr, int64_t tout_millis){\
		varfuture_body_t		*body		= &(future->body);\
		varfuture_trigger_t		*trigger	= body->trigger;\
		int err = 0;\
		\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				if(body->state == varfuture_state_idle){\
					/*idleの場合にはそのまま終わる。*/\
				}\
				else if(body->state == varfuture_state_abrt){\
					/*abrtの場合、errnoを付ける*/\
					err = ECANCELED;\
				}\
				else{\
					/*waitやexecの場合はそのまま待機*/\
					if(varfuture_concurrent_trigger_timedwait(trigger, tout_millis) < 0){\
						err = errno;\
						if(err == ETIMEDOUT){\
							break;\
						}\
					}\
					continue;\
				}\
				break;\
			}while(1);\
			\
			if(ret_ptr != NULL){\
				*ret_ptr = future->ret_val;\
			}\
			if(err_ptr != NULL){\
				*err_ptr = body->err_num;\
			}\
		}varfuture_concurrent_trigger_release(trigger);\
		\
		if(err != 0){\
			errno = err;\
		}\
		return err ? -1:0;\
	}\
	\
	/*待機：returnの取得*/\
	ret_type	VARFUTURE_GET_RETURN_(basename)(VARFUTURE_T_(basename) *future){\
		ret_type ret = ret_init;\
		VARFUTURE_GET_(basename)(future, &ret, NULL);\
		return ret;\
	}\
	\
	/*待機：returnの取得/タイムアウト（ミリ秒）*/\
	ret_type	VARFUTURE_TIMEDGET_RETURN_(basename)(VARFUTURE_T_(basename) *future, int64_t tout_millis){\
		ret_type ret = ret_init;\
		VARFUTURE_TIMEDGET_(basename)(future, &ret, NULL, tout_millis);\
		return ret;\
	}\
	\
	/*待機：エラーの取得*/\
	int			VARFUTURE_GET_ERRNO_(basename)(VARFUTURE_T_(basename) *future){\
		int err_num = 0;\
		VARFUTURE_GET_(basename)(future, NULL, &err_num);\
		return err_num;\
	}\
	\
	/*待機：エラーの取得/タイムアウト（ミリ秒）*/\
	int			VARFUTURE_TIMEDGET_ERRNO_(basename)(VARFUTURE_T_(basename) *future, int64_t tout_millis){\
		int err_num = 0;\
		VARFUTURE_TIMEDGET_(basename)(future, NULL, &err_num, tout_millis);\
		return err_num;\
	}\
	\
	/*再実行*/\
	int			VARFUTURE_REDO_(basename)(VARFUTURE_T_(basename) *future){\
		varfuture_trigger_t	*trigger	= future->body.trigger;\
		varfuture_queue_t	*q			= future->body.queue;\
		\
		int ret = 0;\
		varfuture_concurrent_trigger_acquire(trigger);{\
			do{\
				/*ステート：idleとかabrtじゃないとダメ。*/\
				switch(future->body.state){\
				case varfuture_state_idle:\
				case varfuture_state_abrt:\
					break;\
				default:\
					ret = -1;\
					break;\
				}\
				if(ret){\
					break;\
				}\
				\
				/*モードごとに定めた条件*/\
				switch(future->body.exec_mode){\
				case varfuture_mode_isolate:\
				case varfuture_mode_taskpool:\
					/*exec：関数指定がないとダメ。*/\
					if(future->task.fn_ptr == NULL){\
						ret = -2;\
					}\
					break;\
				default:\
					break;\
				}\
				if(ret){\
					break;\
				}\
				\
				/*ステート遷移：待機*/\
				future->body.state = varfuture_state_wait;\
				\
				/*モード設定が終わったので実行予約に投げ込んでおく*/\
				switch(future->body.exec_mode){\
				case varfuture_mode_taskpool:\
				case varfuture_mode_promise:\
					/*taskpool/promiseの場合は、所属していたキューに再加入する*/\
					if((ret = varfuture_queue_append(q, &(future->body))) != 0){\
						errno = EINVAL;\
					}\
					break;\
				case varfuture_mode_isolate:\
					if((ret = varfuture_concurrent_invoke_isolate(&(future->body))) != 0){\
						/*スレッドの起動に失敗したらそのまま終わる。*/\
						errno = ret;\
						future->body.err_num = ret;\
						future->body.state = varfuture_state_abrt;\
					}\
					break;\
				default:\
					break;\
				}\
			}while(0);\
		}varfuture_concurrent_trigger_release(trigger);\
		return ret;\
	}\
	\
	/*ここから先はpromiseに関する記述*/\
	\
	\
	/*初期化：promiseの初期化*/\
	int			VARPROMISE_INIT_(basename)(VARPROMISE_T_(basename) *promise){\
		varfuture_trigger_t	*future_trigger;\
		varfuture_trigger_t	*modify_trigger;\
		future_trigger = varfuture_concurrent_get_queue_future_trigger();\
		modify_trigger = varfuture_concurrent_get_queue_modify_trigger();\
		return varfuture_queue_init(&(promise->queue), future_trigger, modify_trigger);\
	}\
	\
	/*関連付けたfutureへの通知、戻り値及びエラー値の設定*/\
	int			VARPROMISE_SET_(basename)(VARPROMISE_T_(basename) *promise, ret_type ret, int err_num){\
		varfuture_trigger_t		*trigger	= varfuture_queue_get_trigger(&(promise->queue));\
		VARFUTURE_T_(basename)	*future		= NULL;\
		int ret_num = 0;\
		for(;;){\
			varfuture_concurrent_trigger_acquire(trigger);{\
				future = (VARFUTURE_T_(basename) *)varfuture_queue_get_task(&(promise->queue));\
				if(future == NULL){\
					varfuture_concurrent_trigger_release(trigger);\
					break;\
				}\
				\
				future->body.err_num = err_num;\
				future->body.state = varfuture_state_idle;\
				future->ret_val = ret;\
				varfuture_concurrent_trigger_broadcast(future->body.trigger);\
				ret_num++;\
			}varfuture_concurrent_trigger_release(trigger);\
		}\
		return ret_num;\
	}\
	\

#endif		/* !TYPE_FUTURE_IMPLEMENTS_H */
