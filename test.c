/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/23/14 08:17:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jonesax (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <jnxc_headers/jnxthread.h>
#include "eventhandler.h"
#include <unistd.h>
int test_one_not_complete = 0;
int test_two_not_complete = 0;

//individual test markers
int worker_test_pass = 0;

int worker_callback(event_object *e) {
	worker_test_pass = 1;
	return 0;
}
void *single_event(void *args) {

	jnx_event_handle *h;
	printf("Subscribing...\n");	
	JNX_EVENT_SUBSCRIBE("TestOne",h,worker_callback);
	sleep(2);
	printf("Sending...\n");
	JNX_EVENT_SEND("TestOne","DATA");
	
	while(!worker_test_pass) {

	}
	JNX_EVENT_UNSUBSCRIBE(h);
	test_one_not_complete = 1;
	return 0;
}
int h_test = 0;
int g_test = 0;
int i_test = 0;
int t_test = 0;
int multi_event_temp_callback(event_object *e) {
	printf("Got event [%lu:%s]\n",e->identity,e->evt_type);
	if(strcmp(e->evt_type,"TestOne") == 0) {
		h_test = 1;
	}
	if(strcmp(e->evt_type,"TestTwo") == 0) {
		g_test = 1;
	}
	if(strcmp(e->evt_type,"TestThree") == 0) {
		i_test = 1;
	}
	if(strcmp(e->evt_type,"TestFour") == 0) {
		t_test = 1;
	}
	return 0;
}
void *multi_event(void *args) {

	jnx_event_handle *h;
	JNX_EVENT_SUBSCRIBE("TestOne",h,multi_event_temp_callback);
	jnx_event_handle *g;
	JNX_EVENT_SUBSCRIBE("TestTwo",g,multi_event_temp_callback);
	jnx_event_handle *i;
	JNX_EVENT_SUBSCRIBE("TestThree",i,multi_event_temp_callback);
	jnx_event_handle *j;
	JNX_EVENT_SUBSCRIBE("TestFour",j,multi_event_temp_callback);
	sleep(1);
	JNX_EVENT_SEND("TestOne",NULL);
	JNX_EVENT_SEND("TestTwo",NULL);
	JNX_EVENT_SEND("TestThree",NULL);
	JNX_EVENT_SEND("TestFour",NULL);

	sleep(5);
	assert(h_test);
	assert(g_test);
	assert(i_test);
	assert(t_test);

	test_two_not_complete = 1;
	return 0;
}
int main(int argc, char **argv) {
	
	jnx_event_global_create(10);

	jnx_thread_create_disposable(single_event,NULL);
	
	while(!test_one_not_complete) {
		sleep(1);
	}
	
	jnx_thread_create_disposable(multi_event,NULL);

	while(!test_two_not_complete) {
		sleep(1);
	}

	jnx_event_global_destroy();
	return 0;
}
