#include <stdio.h>
#include <varfuture/primitives.h>

static double average_test(int argc, void *argv[]){
	//printf("%s: %p , %p\n", __func__, arg1, arg2);
	printf("%s: argc is %d, argv is...", __func__, argc);
	int i;
	double ret = 0.0;
	if(argc == 0){
		printf("%s: HEY, ZERO-DIVISION !!\n", __func__);
		return 0.0;
	}
	for(i = 0; i < argc; i++){
		printf("%f, ", *((double*)argv[i]));
		ret += *((double*)argv[i]);
	}
	printf("\n");
	usleep(500*1000);
	printf("%s: done.\n", __func__);
	return ret / (double)argc;
}

static double sum_test(int argc, void *argv[]){
	//printf("%s: %p , %p\n", __func__, arg1, arg2);
	printf("%s: argc is %d, argv is...", __func__, argc);
	int i;
	double ret = 0.0;
	
	for(i = 0; i < argc; i++){
		printf("%f, ", *((double*)argv[i]));
		ret += *((double*)argv[i]);
	}
	printf("\n");
	usleep(500*1000);
	printf("%s: done.\n", __func__);
	return ret;
}

#define ARG1_LEN 5
#define ARG2_LEN 7
void test_double(void *arg){
	double_future_t	d_sample_exec;
	double_future_t	d_sample_isol;
	
	double arg1[ARG1_LEN] = {0.1, 0.2, 0.3, 0.4, 0.5};
	double arg2[ARG2_LEN] = {0.7, 0.6, 0.5, 0.4, 0.5, 0.6, 0.7};
	
	void *varg1[ARG1_LEN];
	void *varg2[ARG2_LEN];
	
	int i;
	
	for(i = 0; i < ARG1_LEN; i++){
		varg1[i] = &arg1[i];
	}
	
	for(i = 0; i < ARG2_LEN; i++){
		varg2[i] = &arg2[i];
	}
	
	printf("%s: start for test\n", __func__);
	printf("%s: [%p], init\n", __func__, &d_sample_exec);
	double_future_init_for_exec(&d_sample_exec);
	
	printf("%s: [%p], init\n", __func__, &d_sample_isol);
	double_future_init_for_isolate(&d_sample_isol);
	//double_future_init_for_exec(&d_sample_isol);
	
	printf("%s: push %p\n", __func__, &d_sample_exec);
	double_future_execv(&d_sample_exec, sum_test, 5, varg1);
	
	printf("%s: push %p\n", __func__, &d_sample_isol);
	double_future_execv(&d_sample_isol, average_test, 7, varg2);
	
	printf("%s: then, wait\n", __func__);
	double res = double_future_get_return(&d_sample_exec) + double_future_get_return(&d_sample_isol);
	printf("%s: per future results are %f, %f\n", __func__, double_future_get_return(&d_sample_exec), double_future_get_return(&d_sample_isol));
	printf("%s: final result is %f\n", __func__, res);
	
	printf("\n\n\n----\n\n\n");
	printf("%s: redo part\n", __func__);
	double_future_redo(&d_sample_exec);
	double_future_redo(&d_sample_isol);
	
	
	printf("%s: then, wait\n", __func__);
	res = double_future_get_return(&d_sample_exec) + double_future_get_return(&d_sample_isol);
	printf("%s: per future results are %f, %f\n", __func__, double_future_get_return(&d_sample_exec), double_future_get_return(&d_sample_isol));
	printf("%s: final result is %f\n", __func__, res);
	
	return;
}
