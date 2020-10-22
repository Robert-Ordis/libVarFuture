/**
 *	\file		varfuture.h
 *	\brief		Completely witchcraft-based future library
 */
#ifndef	VARFUTURE_H
#define	VARFUTURE_H

#include "./type_future_prototypes.h"
#include "./type_future_implements.h"
#include "./primitives.h"
#include "./void_future.h"

int varfuture_global_init(int tp_num);

/*BELOWS ARE EXPERIMENTAL API.*/

/**
 *	\def		varfuture_t
 *	\brief		Templated type of the future object.
 *	\details	Treat this as: "varfuture_t(int)	iFuture;"
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes.
 */
#define	varfuture_t(basename)\
	VARFUTURE_T_(basename)

/**
 *	\def		varfuture_init_for_exec
 *	\brief		Initiate future object for threadpool.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Always returns this.
 *	\remarks	DON'T CALL THIS FOR THE ALREADY STARTED OBJECT.
 *	\remarks	You can also call this as basename_future_init_for_exec(future)
 */
#define	varfuture_init_for_exec(basename, future)\
	VARFUTURE_INIT_FOR_EXEC_(basename)(future)

/**
 *	\def		varfuture_init_for_isolate
 *	\brief		Initiate future object for executing on isolated thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Always returns this.
 *	\remarks	DON'T CALL THIS FOR THE ALREADY STARTED OBJECT.
 *	\remarks	You can also call this as basename_future_init_for_isolate(future)
 */
#define	varfuture_init_for_isolate(basename, future)\
	VARFUTURE_INIT_FOR_ISOLATE_(basename)(future)

/**
 *	\def		varfuture_register_to_promise
 *	\brief		Register the uninitiated future to promise object.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Always returns this.
 *	\remarks	DON'T CALL THIS FOR THE ALREADY STARTED OBJECT.
 *	\remarks	You can also call this as basename_future_register_to_promise(future, promise)
 */
#define	varfuture_register_to_promise(basename, future, promise)\
	VARFUTURE_REGISTER_TO_PROMISE_(basename)(future, promise)

/**
 *	\def		varfuture_exec1
 *	\brief		Reserve the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		func		ret_type(void *)	function you want to invoke.
 *	\param		arg1		(void*) argument for func.
 *	\return	(int)	0 means success. Minus means failure(For detail, get errno, at the exception section.)
 *	\exception	EINVAL	Invlaid future object (e.g. for promise, already exec <= may be EBUSY in the next ver)
 *	\remarks	You can also call this as basename_future_exec1(future, arg1)
 */
#define	varfuture_exec1(basename, future, func, arg1)\
	VARFUTURE_EXEC1_(basename)(future, func, arg1)

/**
 *	\def		varfuture_exec2
 *	\brief		Reserve the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		func		ret_type(void *, void *)	function you want to invoke.
 *	\param		arg1		(void*) argument for func.
 *	\param		arg2		(void*) argument for func
 *	\return	(int)	0 means success. Minus means failure(For detail, get errno, at the exception section.)
 *	\exception	EINVAL	Invlaid future object (e.g. for promise, already exec <= may be EBUSY in the next ver)
 *	\remarks	You can also call this as basename_future_exec2(future, arg1, arg2)
 */
#define	varfuture_exec2(basename, future, func, arg1, arg2)\
	VARFUTURE_EXEC2_(basename)(future, func, arg1, arg2)

/**
 *	\def		varfuture_exec3
 *	\brief		Reserve the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		func		ret_type(void *, void *, void *)	function you want to invoke.
 *	\param		arg1		(void*) argument for func.
 *	\param		arg2		(void*) argument for func
 *	\param		arg3		(void*)	argument for func
 *	\return	(int)	0 means success. Minus means failure(For detail, get errno, at the exception section.)
 *	\exception	EINVAL	Invlaid future object (e.g. for promise, already exec <= may be EBUSY in the next ver)
 *	\remarks	You can also call this as basename_future_exec3(future, arg1, arg2, arg3)
 */
#define	varfuture_exec3(basename, future, func, arg1, arg2, arg3)\
	VARFUTURE_EXEC3_(basename)(future, func, arg1, arg2, arg3)

/**
 *	\def		varfuture_execv
 *	\brief		Reserve the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		func		ret_type(int, void**)	function you want to invoke.
 *	\param		argc		(int) num of argv.
 *	\param		argv		(void **)Array of void* as argument. Size is determined as argc.
 *	\return	(int)	0 means success. Minus means failure(For detail, get errno, at the exception section.)
 *	\exception	EINVAL	Invlaid future object (e.g. for promise, already exec <= may be EBUSY in the next ver)
 *	\remarks	You can also call this as basename_future_execv(future, argc, argv)
 */
#define	varfuture_execv(basename, future, func, argc, argv)\
	VARFUTURE_EXECV_(basename)(future, func, argc, argv)

/**
 *	\def		varfuture_cancel
 *	\brief		Cancel the reserved future.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Minus means failure.
 *	\remarks	You can also call this as basename_future_cancel(future)
 */
#define	varfuture_cancel(basename, future)\
	VARFUTURE_CANCEL_(basename)(future)

/**
 *	\def		varfuture_is_finished
 *	\brief		Reserve the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Minus means failure
 *	\remarks	You can also call this as basename_future_is_finished(future)
 */
#define	varfuture_is_finished(basename, future)\
	VARFUTURE_IS_FINISHED_(basename)(future)

/**
 *	\def		varfuture_get
 *	\brief		Wait and get the result of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		ret_ptr		(int*) pointer to get the return of callback or notified value from the promise.
 *	\param		err_ptr		(int*) pointer to get the errno of callback or notified error from the promise.
 *	\return	(int)	0 means success. Minus means failure
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *	\remarks	You can also call this as basename_future_get(future, ret_ptr, err_ptr)
 */
#define	varfuture_get(basename, future, ret_ptr, err_ptr)\
	VARFUTURE_GET_(basename)(future, ret_ptr, err_ptr)

/**
 *	\def		varfuture_timedget
 *	\brief		Wait and get the result of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		ret_ptr		(int*) pointer to get the return of callback or notified value from the promise.
 *	\param		err_ptr		(int*) pointer to get the errno of callback or notified error from the promise.
 *	\param		tout_millis	(int64_t) Timeout in [msec]
 *	\return	(int)	0 means success. Minus means failure
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *					ETIMEDOUT	timeout.
 *	\remarks	You can also call this as basename_future_timedget(future, ret_ptr, err_ptr, tout_millis)
 */
#define	varfuture_timedget(basename, future, ret_ptr, err_ptr, tout_millis)\
	VARFUTURE_TIMEDGET_(basename)(future, ret_ptr, err_ptr, tout_millis)

/**
 *	\def		varfuture_get_return
 *	\brief		Wait and get the result of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(rettype)	Return value of the callback / Notified value from the promise.
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *	\remarks	You can also call this as basename_future_get_return(future)
 */
#define	varfuture_get_return(basename, future)\
	VARFUTURE_GET_RETURN_(basename)(future)

/**
 *	\def		varfuture_timedget_return
 *	\brief		Wait and get the result of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		tout_millis	(int64_t) Timeout in [msec]
 *	\return	(rettype)	Return value of the callback / Notified value from the promise.
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *					ETIMEDOUT	timeout.
 *	\remarks	You can also call this as basename_future_timedget_return(future, tout_millis)
 */
#define	varfuture_timedget_return(basename, future, tout_millis)\
	VARFUTURE_TIMEDGET_RETURN_(basename)(future, tout_millis)

/**
 *	\def		varfuture_get_error
 *	\brief		Wait and get the errno of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)		errno on the callback / Notified errno from the promise.
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *	\remarks	You can also call this as basename_future_get_error(future)
 */
#define	varfuture_get_errno(basename, future)\
	VARFUTURE_GET_ERRNO_(basename)(future)

/**
 *	\def		varfuture_timedget_error
 *	\brief		Wait and get the result of the callback.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		tout_millis	(int64_t) Timeout in [msec]
 *	\return	(int)		errno on the callback / Notified errno from the promise.
 *	\exception	EINTR	interrupted by system?
 *					ECANCELLED	Canceled by varfuture_cancel
 *					ETIMEDOUT	timeout.
 *	\remarks	You can also call this as basename_future_timedget_error(future, tout_millis)
 */
#define	varfuture_timedget_errno(basename, future, tout_millis)\
	VARFUTURE_TIMEDGET_ERRNO_(basename)(future, tout_millis)

/**
 *	\def		varfuture_redo
 *	\brief		Redo the function call to attached thread.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\return	(int)	0 means success. Minus means failure(For detail, get errno, at the exception section.)
 *	\exception	EINVAL	Invlaid future object (e.g. for promise, already exec <= may be EBUSY in the next ver)
 *	\remarks	If you called for a promise-based future, state will transit to waiting for being notified again.
 *	\remarks	You can also call this as basename_future_redo(future)
 */
#define	varfuture_redo(basename, future)\
	VARFUTURE_REDO_(basename)(future)

/**
 *	\def		varpromise_t
 *	\brief		Templated type of the promise object.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes.
 *	\remarks	You can also treat this as basename_promise_t
 */
#define	varpromise_t(basename)\
	VARPROMISE_T_(basename)

/**
 *	\def		varpromise_init
 *	\brief		Initiate promise object which will notify the regsitered future.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		promise		Instance of promise.
 *	\return	(int)	0 means success. Always returns this.
 *	\remarks	DON'T CALL THIS FOR THE ALREADY STARTED OBJECT.
 *	\remarks	You can also call this as basename_promise_init(promise)
 */
#define	varpromise_init(basename, promise)\
	VARPROMISE_INIT_(basename)(promise)

/**
 *	\def		varpromise_set
 *	\brief		Set the return and errno and notify to the registered futures manually.
 *	\param		basename	basename used on varfuture_primitive_prototypes/varfuture_define_prototypes
 *	\param		future		Instance for handling future.
 *	\param		ret			(ret_type) Return value to send to the futures.
 *	\param		err_num		(int) errno value to send to the futures.
 *	\return	(int)		Number of the futures that received args.
 *	\remarks	You can also call this as basename_promise_set(promise, ret, err_num)
 */
#define	varpromise_set(basename, promise, ret, err_num)\
	VARPROMISE_SET_(basename)(promise, ret, err_num)

#endif	/* !VARFUTURE_H */
