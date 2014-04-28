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

/*  simple test */
int simple_test_var =0;
void simple_test_event_callback(jnx_event *e) {
	simple_test_var = 1;
}
void *simple_test_send(void *args) {
	jnx_event_system_handle *sys_handle = args;
	jnx_event_send(sys_handle,"TEST_EVT",NULL);
	return 0;
}
void simple_test() {

	JNX_LOGC(JLOG_NORMAL,"Simple test: ");
	jnx_event_system_handle *sys_handle = jnx_event_system_create();
	jnx_event_system_listen(sys_handle);
	jnx_event_subscriber *subscriber = jnx_event_subscribe(sys_handle,"TEST_EVT",simple_test_event_callback);
	jnx_thread_create_disposable(simple_test_send,sys_handle);
	time_t s,e;
	time(&s);
	double current = 0;
	float maxtime = 5.0f;
	while(simple_test_var != 1 && current < maxtime) {
		sleep(.5);
		time(&e);
		current = difftime(e,s);
	}
	assert(current < maxtime);
	jnx_event_unsubscribe(sys_handle,subscriber);
	jnx_event_system_destroy(&sys_handle);
	
	jnx_term_printf_in_color(JNX_COL_GREEN," OK\n");
}
/*  complex test */
int complex_test_var0 = 0;
int complex_test_var1 = 0;
int complex_test_var2 = 0;

void complex_test_event_callback0(jnx_event *e) {
	complex_test_var0 = 1;
}
void complex_test_event_callback1(jnx_event *e) {
	complex_test_var1 = 1;
}
void complex_test_event_callback2(jnx_event *e) {
	complex_test_var2 = 1;
}
void *complex_test_send(void *args) {
	jnx_event_system_handle *sys_handle = args;
	jnx_event_send(sys_handle,"TEST_EVT",NULL);
	jnx_event_send(sys_handle,"TEST_EVT2",NULL);
}
void complex_test() {
	JNX_LOGC(JLOG_NORMAL,"Complex test: ");
	jnx_event_system_handle *sys_handle = jnx_event_system_create();
	jnx_event_system_listen(sys_handle);


	jnx_event_subscriber *subscriber0 = jnx_event_subscribe(sys_handle,"TEST_EVT",complex_test_event_callback0);
	jnx_event_subscriber *subscriber1 = jnx_event_subscribe(sys_handle,"TEST_EVT",complex_test_event_callback1);
	jnx_event_subscriber *subscriber2 = jnx_event_subscribe(sys_handle,"TEST_EVT2",complex_test_event_callback2);

	jnx_thread_create_disposable(complex_test_send,sys_handle);
	time_t s,e;
	time(&s);
	double current = 0;
	float maxtime = 5.0f;
	while(complex_test_var0 != 1 && complex_test_var1 != 1 && complex_test_var2 != 1 && current < maxtime) {
		sleep(.5);
		time(&e);
		current = difftime(e,s);
	}
	assert(current < maxtime);
	jnx_event_unsubscribe(sys_handle,subscriber0);
	jnx_event_unsubscribe(sys_handle,subscriber1);
	jnx_event_unsubscribe(sys_handle,subscriber2);
	jnx_event_system_destroy(&sys_handle);
	jnx_term_printf_in_color(JNX_COL_GREEN," OK\n");
}
int main(int argc, char **argv) {

	JNX_LOGC(JLOG_NORMAL,"Starting event tests...\n");	
	simple_test();
	complex_test();
	printf("Jnx event handler system:");
    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
	return 0;
}
