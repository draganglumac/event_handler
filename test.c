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
int multi_event_temp_callback(event_object *e) {

	return 0;
}
void *multi_event(void *args) {

	jnx_event_handle *h;
	JNX_EVENT_SUBSCRIBE("TestOne",h,multi_event_temp_callback);
	jnx_event_handle *g;
	JNX_EVENT_SUBSCRIBE("TestOne",g,multi_event_temp_callback);
	jnx_event_handle *i;
	JNX_EVENT_SUBSCRIBE("TestTwo",i,multi_event_temp_callback);
	jnx_event_handle *j;
	JNX_EVENT_SUBSCRIBE("TestThree",j,multi_event_temp_callback);

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
