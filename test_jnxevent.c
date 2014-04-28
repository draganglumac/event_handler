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
#include <jnxc_headers/jnxterm.h>
#include <jnxc_headers/jnxlog.h>
#include "src/jnxevent.h"
#include <unistd.h>
#include <time.h>

int simple_test =0;
void test_event_callback(jnx_event *e) {

	simple_test = 1;
}

void *test_send(void *args) {

	JNX_LOGC(JLOG_NORMAL,"Sending event!\n");
	jnx_event_system_handle *sys_handle = args;
	jnx_event_send(sys_handle,"TEST_EVT",NULL);

	return 0;
}
int main(int argc, char **argv) {

	JNX_LOGC(JLOG_NORMAL,"Starting event tests...\n");	


	jnx_event_system_handle *sys_handle = jnx_event_system_create();

	jnx_event_system_listen(sys_handle);

	jnx_event_subscriber *subscriber = jnx_event_subscribe(sys_handle,"TEST_EVT",test_event_callback);

	jnx_thread_create_disposable(test_send,sys_handle);
	while(simple_test != 1) {

		sleep(.5);
	}

	jnx_event_system_destroy(&sys_handle);

	printf("Jnx event handler system:");
    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
	return 0;
}
