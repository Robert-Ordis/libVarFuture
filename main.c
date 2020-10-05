#include <varfuture/varfuture.h>
#include <varfuture/void_future.h>
#include <varfuture/primitives.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

void test_int(void *arg);
void test_double(void *arg);

void tekitou(void *arg){
	usleep(1*1000*1000);
	printf("%s[%p]: test invoke\n", __func__, arg);
	printf("%s[%p]: get queue from cb->%p\n", __func__, arg, varfuture_concurrent_get_threadpool_queue());
	//free(arg);
}

void tekitou2(void *arg1, void *arg2){
	usleep(1*1000*1000);
	printf("%s[%p, %p]: test invoke\n", __func__, arg1, arg2);
	printf("%s[%p, %p]: get queue from cb->%p\n", __func__, arg1, arg2, varfuture_concurrent_get_threadpool_queue());
}

void tekitou3(void *arg1, void *arg2, void *arg3){
	usleep(1*1000*1000);
	printf("%s[%p, %p, %p]: test invoke\n", __func__, arg1, arg2, arg3);
	printf("%s[%p, %p, %p]: get queue from cb->%p\n", __func__, arg1, arg2, arg3, varfuture_concurrent_get_threadpool_queue());
}

void tekitouv_int(int argc, void *argv[]){
	usleep(1*1000*1000);
	printf("%s[%d, %p] args is ...", __func__, argc, argv);
	int i;
	int sum = 0;
	for(i = 0; i < argc; i++){
		sum+=(int)argv[i];
		printf("%p ", argv[i]);
	}
	printf("->%d\n", sum);
}

void tekitouv_str(int argc, void *argv[]){
	usleep(1*1000*1000);
	printf("%s[%d, %p] args is ...", __func__, argc, argv);
	int i;
	for(i = 0; i < argc; i++){
		printf("%s ", argv[i]);
	}
	printf("\n");
}

void *tekitouPromise(void *arg){
	usleep(1*1000*1000);
	
	void_promise_t *promise = (void_promise_t*)arg;
	printf("%s[%p]: invoke for promise\n", __func__, promise);

	void_promise_set(promise, NULL, ETIMEDOUT);
	return NULL;
}

void tekitou_thread(void *arg){
	char one[] = "aiueo";
	char two[] = "akebb";
	char thr[] = "aiebe";
	
	void *chars[] = {one, two, thr, "four!"};
	
	void *arr[] = {1,2,3,4,5};
	
	printf("\n\n\n%s: global init complete\n", __func__);
	void_future_t	sample, sample2;
	void_future_init_for_exec(&sample);
	void_future_init_for_isolate(&sample2);
	
	void_future_exec1(&sample, tekitou, &sample);
	//void_future_exec3(&sample2, tekitou3, &sample2, 0x30, 0xFF);
	printf("%p\n", arr);
	//void_future_execv(&sample2, tekitouv_str, 4, chars);
	void_future_execv(&sample2, tekitouv_int, 5, arr);
	//sched_yield();
	void_future_cancel(&sample2);
	
	
	printf("%s: wait for %p return\n", __func__, &sample);
	void_future_get_return(&sample);
	int err = errno;
	printf("%s: return end->%s/%s\n", __func__, strerror(err), strerror(void_future_get_errno(&sample)));
	
	printf("%s: wait for %p return\n", __func__, &sample2);
	void_future_get_return(&sample2);
	err = errno;
	printf("%s: return end->%s/%s\n", __func__, strerror(err), strerror(void_future_get_errno(&sample2)));
	
	printf("\n\n\n=====\n\n\n");
	
	printf("%s: reboot two futures\n", __func__);
	printf("%s: one[%p]->%d\n", __func__, &sample, void_future_redo(&sample));
	printf("%s: two[%p]->%d\n", __func__, &sample2, void_future_redo(&sample2));
	
	printf("%s: wait for %p return\n", __func__, &sample);
	void_future_timedget_return(&sample, 500);
	err = errno;
	printf("%s: return end->%s/%s\n", __func__, strerror(err), strerror(void_future_get_errno(&sample)));
	
	printf("%s: wait for %p return\n", __func__, &sample2);
	void_future_timedget_return(&sample2, 500);
	err = errno;
	printf("%s: return end->%s/%s\n", __func__, strerror(err), strerror(void_future_get_errno(&sample2)));
	
	printf("\n\n\n=====\n\n\n");
	
	void_promise_t	prom;
	
	printf("%s: promise init\n", __func__);
	void_promise_init(&prom);
	
	printf("%s: register to proimise\n", __func__);
	void_future_register_to_promise(&sample, &prom);
	
	
	pthread_t t;
	printf("%s: invoke promise thread\n", __func__);
	pthread_create(&t, NULL, &tekitouPromise, &prom);
	printf("%s: invoke isolate task again\n", __func__);
	void_future_redo(&sample2);
	printf("%s: errno from prom is %d\n", __func__, void_future_get_errno(&sample));
	printf("%s: errno from isol is %d\n", __func__, void_future_get_errno(&sample2));
	pthread_join(t, NULL);
	printf("\n\n\n\n\n%s: then, end of the exam of void_future\n\n\n\n\n", __func__);
}

int main(int argc, char *argv[]){
	printf("%s: global init\n", __func__);
	varfuture_global_init(4);
	
	pthread_t at;
	//pthread_create(&at, NULL, tekitou_thread, NULL);
	//pthread_create(&at, NULL, tekitou_thread, NULL);
	tekitou_thread(NULL);
	
	//pthread_join(at, NULL);
	test_int(NULL);
	test_double(NULL);
	return 0;
}

