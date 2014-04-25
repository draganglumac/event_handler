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

int multi_recp_callback0(event_object *e) { 
	JNX_LOGC(JLOG_DEBUG,"Firing multi_recp_callback0  Got data[%s]\n",e->evt_data);
	assert(strcmp(e->evt_data,"Hello") == 0);
   	has_fired_recp_cb0 = 1;
	return 0;
}
int multi_recp_callback1(event_object *e) {
	JNX_LOGC(JLOG_DEBUG,"Firing multi_recp_callback1  Got data[%s]\n",e->evt_data);
   	assert(strcmp(e->evt_data,"Hello") == 0);
	has_fired_recp_cb1 = 1;
	return 0;
}
int multi_recp_callback2(event_object *e) {
	JNX_LOGC(JLOG_DEBUG,"Firing multi_recp_callback2  Got data[%s]\n",e->evt_data);
   	assert(strcmp(e->evt_data,"Hello") == 0);
	has_fired_recp_cb2 = 1;
	return 0;
}
int multi_recp_callback3(event_object *e) { 
	JNX_LOGC(JLOG_DEBUG,"Firing multi_recp_callback3  Got data[%s]\n",e->evt_data);
   	assert(strcmp(e->evt_data,"Hello") == 0);
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
int main(int argc, char **argv) {
	
	jnx_event_global_create(10);

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


	jnx_event_global_destroy();
	printf("Jnx event handler system:");
    jnx_term_printf_in_color(JNX_COL_GREEN, "  OK\n");
	return 0;
}
