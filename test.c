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
void *worker(void *args) {

	sleep(1);

	JNX_EVENT_SEND("BLAH!","DATA");

	sleep(1);
	return 0;
}

int callback(event_object *e) {

	printf("Got event of type %s\n",e->evt_type);
	return 0;
}



int main(int argc, char **argv) {
	
	jnx_event_global_create(10);

	JNX_EVENT_SUBSCRIBE("BLAH!",callback);

	jnx_thread_create_disposable(worker,NULL);

	sleep(4);
	jnx_event_global_destroy();
	return 0;
}
