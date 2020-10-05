/**
 *	\file	private/pertype_functions_definition.h
 *	\brief	タイプごとの関数宣言のベース部分。
 *
 *
 */

#ifndef		PRIVATE_PERTYPE_FUNCTION_DEFINITION_H
#define		PRIVATE_PERTYPE_FUNCTION_DEFINITION_H

//type_future部分

//構造体名（future）
#define		VARFUTURE_T_(basename)						basename##_future_t

//コールバック関連
#define		VARFUTURE_FUNC1_CB_(basename)				basename##_future_func1_cb
#define		VARFUTURE_FUNC2_CB_(basename)				basename##_future_func2_cb
#define		VARFUTURE_FUNC3_CB_(basename)				basename##_future_func3_cb
#define		VARFUTURE_FUNCV_CB_(basename)				basename##_future_funcv_cb

#define		VARFUTURE_EXEC_INVOKE_(basename)			basename##_future_exec_invoke_

#define		VARFUTURE_TASK_(basename)					basename##_future_task

#define		VARFUTURE_INIT_FOR_EXEC_(basename)			basename##_future_init_for_exec

#define		VARFUTURE_INIT_FOR_ISOLATE_(basename)		basename##_future_init_for_isolate

#define		VARFUTURE_REGISTER_TO_PROMISE_(basename)	basename##_future_register_to_promise

#define		VARFUTURE_EXEC1_(basename)					basename##_future_exec1

#define		VARFUTURE_EXEC2_(basename)					basename##_future_exec2

#define		VARFUTURE_EXEC3_(basename)					basename##_future_exec3

#define		VARFUTURE_EXECV_(basename)					basename##_future_execv

#define		VARFUTURE_CANCEL_(basename)					basename##_future_cancel

#define		VARFUTURE_IS_FINISHED_(basename)			basename##_future_is_finished

#define		VARFUTURE_GET_(basename)					basename##_future_get

#define		VARFUTURE_TIMEDGET_(basename)				basename##_future_timedget

#define		VARFUTURE_GET_RETURN_(basename)				basename##_future_get_return

#define		VARFUTURE_TIMEDGET_RETURN_(basename)		basename##_future_timedget_return

#define		VARFUTURE_GET_ERRNO_(basename)				basename##_future_get_errno

#define		VARFUTURE_TIMEDGET_ERRNO_(basename)			basename##_future_timedget_errno

#define		VARFUTURE_REDO_(basename)					basename##_future_redo

//type_promise部分

//構造体名（promise）
#define		VARPROMISE_T_(basename)						basename##_promise_t

#define		VARPROMISE_INIT_(basename)					basename##_promise_init

#define		VARPROMISE_SET_(basename)					basename##_promise_set




#endif		/* !PRIVATE_PERTYPE_FUNCTION_DEFINITION_H */
