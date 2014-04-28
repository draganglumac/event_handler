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
int test_one_not_complete = 0;
int test_two_not_complete = 0;
int test_three_not_complete = 0;
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
	printf("Got event [%lu:%d]\n",e->identity,e->evt_type);
	if(jnx_event_is_of_type("TestOne",e)) {
		h_test = 1;
	}
	if(jnx_event_is_of_type("TestTwo",e)) {
		g_test = 1;
	}
	if(jnx_event_is_of_type("TestThree",e)) {
		i_test = 1;
	}
	if(jnx_event_is_of_type("TestFour",e)) {
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

	JNX_EVENT_UNSUBSCRIBE(h);
	JNX_EVENT_UNSUBSCRIBE(g);
	JNX_EVENT_UNSUBSCRIBE(i);
	JNX_EVENT_UNSUBSCRIBE(j);
	test_two_not_complete = 1;
	return 0;
}


int has_fired_recp_cb0 = 0;
int has_fired_recp_cb1 = 0;
int has_fired_recp_cb2 = 0;
int has_fired_recp_cb3 = 0;
jnx_thread_mutex thr_lock;

int multi_recp_callback0(event_object *e) { 
	jnx_thread_lock(&thr_lock);
   	has_fired_recp_cb0 = 1;
	return 0;
}
int multi_recp_callback1(event_object *e) {
	has_fired_recp_cb1 = 1;
	return 0;
}
int multi_recp_callback2(event_object *e) {
	has_fired_recp_cb2 = 1;
	return 0;
}
int multi_recp_callback3(event_object *e) { 
	has_fired_recp_cb3 = 1;
	return 0;
}


void* multi_recipient(void *args) {

	jnx_event_handle *m0;
	JNX_EVENT_SUBSCRIBE("multi_fire",m0,multi_recp_callback0);
	jnx_event_handle *m1;
	JNX_EVENT_SUBSCRIBE("multi_fire",m1,multi_recp_callback1);
	jnx_event_handle *m2;
	JNX_EVENT_SUBSCRIBE("multi_fire",m2,multi_recp_callback2);
	jnx_event_handle *m3;
	JNX_EVENT_SUBSCRIBE("multi_fire",m3,multi_recp_callback3);

	time_t s,e;
	time(&s);

	uint8_t *payload = JNX_MEM_MALLOC(8);
	strcpy(payload,"Hello");
	JNX_EVENT_SEND("multi_fire",payload);

	double duration = 0;
	double max_wait_time = 5.0f;
	while((!has_fired_recp_cb0 && !has_fired_recp_cb1 && !has_fired_recp_cb2 && !has_fired_recp_cb3) && duration < max_wait_time ){
	
		time(&e);
		duration = difftime(e,s);	
	}
	assert(duration < max_wait_time);
	JNX_MEM_FREE(payload);
	test_three_not_complete = 1;
	return 0;
}

int thread1_handler_called = 0;
int thread1_handler(event_object *e) {
	thread1_handler_called++;
	return 0;
}
void *thread1_loop(void *args) {
	jnx_event_handle *h1, *h2;
	JNX_EVENT_SUBSCRIBE("event1", h1, thread1_handler);
	JNX_EVENT_SUBSCRIBE("event2", h2, thread1_handler);
	
	time_t start, now;
	double duration = 0.0, max = 10.0;
	
	time(&start);
	while (duration < max) {
		if (thread1_handler_called == 2) {
			break;
		}
		time(&now);
		duration += (now - start);
		sleep(1);
	}
	JNX_EVENT_UNSUBSCRIBE(h1);
	JNX_EVENT_UNSUBSCRIBE(h2);
	
	return NULL;
}
int thread2_handler_called = 0;
int thread2_handler(event_object *e) {
	thread2_handler_called++;
	return 0;
}
void *thread2_loop(void *args) {
	jnx_event_handle *h1, *h3;
	JNX_EVENT_SUBSCRIBE("event1", h1, thread2_handler);
	JNX_EVENT_SUBSCRIBE("event3", h3, thread2_handler);

	time_t start, now;
	double duration = 0.0, max = 10.0;
	
	time(&start);
	while (duration < max) {
		if (thread2_handler_called == 2) {
			break;
		}
		time(&now);
		duration += (now - start);
		sleep(1);
	}
	JNX_EVENT_UNSUBSCRIBE(h1);
	
	return NULL;
}

void test_events_in_multiple_threads_and_no_payload() {
	JNX_LOGC(JLOG_DEBUG,"test_events_in_multiple_threads_and_no_payload -");
	
	jnx_thread_create_disposable(thread1_loop, NULL);
	jnx_thread_create_disposable(thread2_loop, NULL);

	JNX_EVENT_SEND("event1", NULL);
	JNX_EVENT_SEND("event2", NULL);
	JNX_EVENT_SEND("event3", NULL);

	while (thread1_handler_called != 2 && thread2_handler_called != 2) {
		sleep(1);
	}

    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
}



void test_destructor_thrashing() {
	JNX_LOGC(JLOG_NORMAL,"test_destructor_thrashing -");
	int y = 0, x = 5;

	for(y=0; y < x; ++y) {
		jnx_event_global_create();
		jnx_event_global_destroy();
	}

    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
}
int main(int argc, char **argv) {

	JNX_LOGC(JLOG_NORMAL,"Starting event tests...\n");	
	
	jnx_event_global_create();

	jnx_thread_create_disposable(single_event,NULL);
	
    time_t s,e;
    time(&s);
	double duration = 0;
	double max_test_timeout = 10.0f;
	while(!test_one_not_complete && duration < max_test_timeout) {
		time(&e);
		duration = difftime(e,s);	
	}
	JNX_LOGC(JLOG_DEBUG,"Test one took %fs\n",duration);
	assert(duration < max_test_timeout);

	
    time(&s);
	duration = 0;
	jnx_thread_create_disposable(multi_event,NULL);

	while(!test_two_not_complete && duration < max_test_timeout) {
		time(&e);
		duration = difftime(e,s);	
	}

	JNX_LOGC(JLOG_DEBUG,"Test two took %fs\n",duration);
	assert(duration < max_test_timeout);

	time(&s);
	duration = 0;
	jnx_thread_create_disposable(multi_recipient,NULL);

	while(!test_three_not_complete && duration < max_test_timeout) {
		time(&e);
		duration = difftime(e,s);	
	}

	JNX_LOGC(JLOG_DEBUG,"Test three took %fs\n",duration);
	assert(duration < max_test_timeout);

	test_events_in_multiple_threads_and_no_payload();

	jnx_event_global_destroy();
	
	test_destructor_thrashing();
	
	
	printf("Jnx event handler system:");
    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
	return 0;
}
