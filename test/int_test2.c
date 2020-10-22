#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <varfuture/varfuture.h>

static int addition_test(void *arg1, void *arg2){
	printf("%s: %p + %p\n", __func__, arg1, arg2);
	usleep(500*1000);
	printf("%s: done.\n", __func__);
	return (int)arg1 + (int)arg2;
}

static void* promise_test(void *arg){
	printf("%s: start promise task\n", __func__);
	usleep(750 * 1000);
	printf("%s: wakeup promise task\n", __func__);
	int_promise_t *prom = (int_promise_t *)arg;
	int_promise_set(prom, 256, 0);
	return 0;
}

void test_int2(void *arg){
	varfuture_t(int)	int_sample_exec;
	varfuture_t(int)	int_sample_isol;
	varfuture_t(int)	int_sample_prom;
	
	varpromise_t(int)	promise_exam;
	
	pthread_t		th;
	
	varpromise_init(int, &promise_exam);
	
	printf("%s: start for test\n", __func__);
	printf("%s: [%p], init\n", __func__, &int_sample_exec);
	varfuture_init_for_exec(int, &int_sample_exec);
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_exec, varfuture_is_finished(int, &int_sample_exec));
	
	printf("%s: [%p], init\n", __func__, &int_sample_isol);
	//int_future_init_for_isolate(&int_sample_isol);
	varfuture_init_for_exec(int, &int_sample_isol);
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_isol, varfuture_is_finished(int, &int_sample_isol));
	
	varfuture_register_to_promise(int, &int_sample_prom, &promise_exam);
	pthread_create(&th, NULL, promise_test, &promise_exam);
	
	printf("%s: push %p\n", __func__, &int_sample_exec);
	varfuture_exec2(int, &int_sample_exec, addition_test, 3, 5);
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_exec, varfuture_is_finished(int, &int_sample_exec));
	
	printf("%s: push %p\n", __func__, &int_sample_isol);
	varfuture_exec2(int, &int_sample_isol, addition_test, 5, 8);
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_isol, varfuture_is_finished(int, &int_sample_isol));
	
	printf("%s: then, wait\n", __func__);
	//printf("%s: [%p, %p] return is %d, %d\n", __func__, &int_sample_exec, &int_sample_isol, int_future_get_return(&int_sample_exec), int_future_get_return(&int_sample_isol));
	printf("%s: sum of 2 is %d\n", __func__, varfuture_get_return(int, &int_sample_isol) + varfuture_get_return(int, &int_sample_exec));
	printf("%s: and, final resultis %d\n", __func__, varfuture_get_return(int, &int_sample_isol) + varfuture_get_return(int, &int_sample_exec) + varfuture_get_return(int, &int_sample_prom));
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_exec, varfuture_is_finished(int, &int_sample_exec));
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_isol, varfuture_is_finished(int, &int_sample_isol));
	pthread_join(th, NULL);
	printf("%s: redo promise. (creating a thread manually.\n", __func__);
	int_future_redo(&int_sample_prom);
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_prom, varfuture_is_finished(int, &int_sample_prom));
	pthread_create(&th, NULL, promise_test, &promise_exam);
	printf("%s: done. result is %d\n", __func__, varfuture_get_return(int, &int_sample_prom));
	printf("%s: [%p], gettable->%d\n", __func__, &int_sample_prom, varfuture_is_finished(int, &int_sample_prom));
	pthread_join(th, NULL);
	
	int err;
	printf("%s: then, cancel test of promise\n", __func__);
	varfuture_redo(int, &int_sample_prom);
	varfuture_cancel(int, &int_sample_prom);
	varfuture_get(int, &int_sample_prom, NULL, NULL);
	err = errno;
	printf("%s: all done\n", __func__);
	printf("%s: and cancel test result is %s\n", __func__, strerror(err));
	return;
}
