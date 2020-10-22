/**
 *	\file		engine_threadpool.c
 *	\brief		エンジン部分のうち、並列処理関連の雑多な実装についてまとめたもの
 *	
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "varfuture/varfuture.h"
#include "varfuture/private/engine/trigger.h"
#include "varfuture/private/body.h"
#include "varfuture/private/engine/threadpool.h"
#include "varfuture/private/engine/concurrent_api.h"
#include "varfuture/varfuture_depend.h"


/**
 *	\fn			varfuture_concurrent_init
 *	\brief		初期化。それに伴ってバックグラウンドスレッドの起動
 *	\param		(int)tp_num		:スレッド数
 *	\return	(int)起動できた数。-1, -2でcond初期化エラー。-3でmalloc/-4以下でスレッド起動エラー。かつerrno格納。
 *	\remarks	tp_numは1以下の場合では強制的にデフォルト数起動とする
 */
int	varfuture_global_init(int tp_num){
	return varfuture_concurrent_init(tp_num);
}

