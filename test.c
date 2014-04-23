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
int test_not_complete = 0;

//individual test markers
int worker_test_pass = 0;

int worker_callback(event_object *e) {
	worker_test_pass = 1;
	return 0;
}
void *worker(void *args) {

	jnx_event_handle *h;
	printf("Subscribing...\n");	
	JNX_EVENT_SUBSCRIBE("TestOne",h,worker_callback);
	sleep(2);
	printf("Sending...\n");
	JNX_EVENT_SEND("TestOne","DATA");
	while(!worker_test_pass) {

	}
	test_not_complete = 1;
	return 0;
}

int main(int argc, char **argv) {
	
	jnx_event_global_create(10);

	jnx_thread_create_disposable(worker,NULL);
	

	while(!test_not_complete) {
		sleep(1);
	}

	jnx_event_global_destroy();
	return 0;
}
