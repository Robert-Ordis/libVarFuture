/**
 *	\file	private/inline_processes.h
 *	\brief	内部処理の共通部分
 *
 *
 */

#ifndef		VARFUTURE_PRIVATE_INLINE_PROCESSES_H
#define		VARFUTURE_PRIVATE_INLINE_PROCESSES_H

//関数指定モードにおける起動前検査
#define	VARFUTURE_BODY_BEFORE_EXEC_(body, ret)\
	do{\
		\
		ret = 0;\
		/*動作中の状態で設定しようとしても駄目です*/\
		switch((body)->state){\
		case varfuture_state_idle:\
		case varfuture_state_abrt:\
			break;\
		default:\
			ret = -1;\
			break;\
		}\
		\
		if(ret < 0)\
			break;\
		\
		/*関数指定実行モードでない場合NGです*/\
		switch((body)->exec_mode){\
		case varfuture_mode_taskpool:\
		case varfuture_mode_isolate:\
			break;\
		default:\
			ret = -2;\
			break;\
		}\
	}while(0)\
	\

//関数指定モードにおける起動処理
#define	VARFUTURE_BODY_EXEC_INVOKE_(body, ret)\
	do{\
		ret = 0;\
		\
		/*モード設定が終わったので実行予約に投げ込んでおく*/\
		switch((body)->exec_mode){\
		case varfuture_mode_taskpool:\
			if((ret = varfuture_queue_append((body)->queue, (body))) != 0){\
				errno = EINVAL;\
			}\
			break;\
		case varfuture_mode_isolate:\
			if((ret = varfuture_concurrent_invoke_isolate((body))) != 0){\
				/*スレッドの起動に失敗したらそのまま終わる。*/\
				errno = ret;\
				(body)->err_num = ret;\
				(body)->state = varfuture_state_abrt;\
			}\
			break;\
		default:\
			break;\
		}\
	}while(0)\
	\
	
#endif		/* !VARFUTURE_PRIVATE_INLINE_PROCESSES_H */
