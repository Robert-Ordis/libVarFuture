/**
 *	\file		private/varfuture_trigger_pthread.realtime.h
 *	\brief		varfutureのロック/シグナル系統に関してpthread依存な部分をまとめる
 *	\remarks	ラッパー関数的な感じでやりたいんだけど、いかんせんノウハウがないなあ
 */

#ifndef	PRIVATE_VARFUTURE_ENGINE_TRIGGER_PTHREAD_H
#define	PRIVATE_VARFUTURE_ENGINE_TRIGGER_PTHREAD_H

#include <time.h>
#include <sys/time.h>
#include <pthread.h>

typedef pthread_t				varfuture_thread_t;
typedef pthread_mutex_t		varfuture_lock_t;
typedef pthread_cond_t			varfuture_cond_t;
typedef pthread_condattr_t		varfuture_condattr_t;

#define	VARFUTURE_LOCK_GLOBAL_INIT PTHREAD_MUTEX_INITIALIZER;
#define	VARFUTURE_COND_GLOBAL_INIT PTHREAD_COND_INITIALIZER;

#warning "In any reason, using "varfuture_timedget" family in CLOCK_REALTIME is NOT RECOMMENDED."
#warning "I provide "varfuture_is_finished" instead."

#define varfuture_lock_init(lock, err_ret)\
	do{\
		*(err_ret) = pthread_mutex_init(lock, NULL);\
	}while(0)\
	
#define	varfuture_thread_current	pthread_self

#define	varfuture_thread_equal		pthread_equal
	
#define	varfuture_lock_acquire(lock, err_ret)\
	do{\
		/*printf("%s: lock with %p\n", __func__, lock);*/\
		*(err_ret) = pthread_mutex_lock(lock);\
		/*printf("%s: ->%d\n", __func__, *err_ret);*/\
	}while(0)\
	
#define	varfuture_lock_release(lock, err_ret)\
	do{\
		*(err_ret) = pthread_mutex_unlock(lock);\
		/*printf("%s: unlock with %p->%d\n", __func__, lock, *err_ret);*/\
	}while(0)\
	
#define varfuture_cond_init(cond, condattr, err_ret, step)\
	do{\
		*(step) = 0;\
		*(err_ret) = 0;\
		varfuture_condattr_t *pattr = condattr;\
		if(pattr != NULL){\
			if((*(err_ret) = pthread_condattr_init(pattr)) < 0){\
				*(step) = -1;\
				break;\
			}\
		}\
		*(err_ret) = pthread_cond_init(cond, pattr);\
	}while(0)\

//実際のロック変数がグローバルであれ、スレッドプール所持であれ、とりあえずアサイン形式で行う
#define	varfuture_trigger_assign(trigger, p_lock, p_cond)\
	do{\
		(trigger)->lock = p_lock;\
		(trigger)->cond = p_cond;\
	} while(0)\

//普通のロック
#define	varfuture_trigger_acquire(trigger, err_ret)\
	do{\
		/*printf("%s: acquire(%p)->(%p)\n", __func__, trigger, (trigger)->lock);*/\
		*(err_ret) = pthread_mutex_lock((trigger)->lock);\
	}while(0)\
	
//普通のアンロック
#define	varfuture_trigger_release(trigger, err_ret)\
	do{\
		*(err_ret) = pthread_mutex_unlock((trigger)->lock);\
		/*printf("%s: release(%p)->(%p)\n", __func__, trigger, (trigger)->lock);*/\
	}while(0)\
	
//ロックのもとで待機する
#define	varfuture_trigger_wait(trigger, err_ret)\
	do{\
		/*printf("%s: wait(%p)->(%p)\n", __func__, trigger, (trigger)->lock);*/\
		*(err_ret) = pthread_cond_wait((trigger)->cond, (trigger)->lock);\
	}while(0)\
	
//ロックのもとで時間制限付きで待機する
#define varfuture_trigger_timedwait(trigger, millis, err_ret)\
	do{\
		/*printf("%s: timedwait(%p)->(%p)\n", __func__, trigger, (trigger)->lock);*/\
		int64_t tv_nsec_add = millis < 0 ? 0:millis;\
		tv_nsec_add = tv_nsec_add * 1000 * 1000;\
		time_t add_sec = 0;\
		struct timespec tsp;\
		*(err_ret) = 0;\
		if(tv_nsec_add <= 0){\
			varfuture_trigger_release(trigger, err_ret);\
			varfuture_trigger_acquire(trigger, err_ret);\
			break;\
		}\
		/*printf("%s: timed wait in REALTIME...(omg)\n", __func__);*/\
		if(clock_gettime(CLOCK_REALTIME, &tsp) < 0)\
			*(err_ret) = errno;\
		tsp.tv_nsec += tv_nsec_add;\
		add_sec = (int)(tsp.tv_nsec / (1000*1000*1000));\
		tsp.tv_sec += add_sec;\
		tsp.tv_nsec -= (add_sec * 1000*1000*1000);\
		*(err_ret) = pthread_cond_timedwait((trigger)->cond, (trigger)->lock, &tsp);\
		/*驚くべきことに、pthreadはerrno使っていなかった……*/\
	}while(0)\
	
//ロックのもとで待機している子を誰か起こす
#define	varfuture_trigger_signal(trigger, err_ret)\
	do{\
		*(err_ret) = pthread_cond_signal((trigger)->cond);\
	}while(0)\
	
//ロックのもとで待機している子を全て起こす
#define	varfuture_trigger_broadcast(trigger, err_ret)\
	do{\
		*(err_ret) = pthread_cond_broadcast((trigger)->cond);\
	}while(0)\

#endif	/* !PRIVATE_VARFUTURE_ENGINE_TRIGGER_PTHREAD_H */
