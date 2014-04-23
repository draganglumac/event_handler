/*
 * =====================================================================================
 *
 *       Filename:  eventhandler.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/23/14 08:19:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jonesax (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <jnxc_headers/jnxthread.h>
#include <jnxc_headers/jnxqueue.h>
#include <jnxc_headers/jnxlog.h>
#include "eventhandler.h"
#include <time.h>
jnx_thread_mutex evt_lock;
jnx_thread_mutex queue_lock;
pthread_t evt_thr;
pthread_mutex_t event_lock;
static int exiting = 0;
static jnx_list *subscription_list = NULL;
static jnx_queue *event_queue = NULL;

unsigned long jnx_event_identity_create() {
	return rand();
}
jnx_event *jnx_event_handle_create(uint8_t *evt_type,jnx_event_callback c) {
	assert(evt_type);
	assert(c);
	jnx_event *e = JNX_MEM_MALLOC(sizeof(jnx_event));
	e->c = c;
	e->evt_type = strndup(evt_type,strlen(evt_type));
	e->identity = jnx_event_identity_create();	
	JNX_LOGC(JLOG_NORMAL,"Generated eventhandler with ID:%ld\n",e->identity);
	return e;
}
void jnx_event_subscribe(jnx_event *e) {
	jnx_list_add(subscription_list,e);
}
event_object *jnx_event_object_create(uint8_t *evt_type,void *data) {
	JNX_LOGC(JLOG_NORMAL,"Creating event object\n");
	event_object *eo = JNX_MEM_MALLOC(sizeof(event_object));
	eo->evt_type = strndup(evt_type,strlen(evt_type));
	eo->evt_data = data;
	eo->identity = jnx_event_identity_create();
	JNX_LOGC(JLOG_NORMAL,"Generated event with ID:%lu\n",eo->identity);
	return eo;
}
void jnx_event_update_subscribers(event_object *e) {
	assert(e);	
	jnx_thread_lock(&evt_lock);
	jnx_node *head = subscription_list->head,
			 *reset = subscription_list->head;
	while(head) {
		jnx_event *je = head->_data;
		je->c(e);	
		head = head->next_node;
	}
	head = reset;
	jnx_thread_unlock(&evt_lock);
	JNX_LOGC(JLOG_NORMAL,"Queue length %d\n",event_queue->list->counter);
}
int jnx_event_send(event_object *e) {
	if(exiting) {
		JNX_MEM_FREE(e);
		return 1;
	}
	jnx_thread_lock(&queue_lock);
	jnx_queue_push(event_queue,e);
	jnx_thread_unlock(&queue_lock);
	JNX_MEM_FREE(e);
	return 0;
}
void *jnx_event_mainloop(void *args) {

	struct timespec tim, tim2;
	tim.tv_sec=0;
	tim.tv_nsec=250000000L;

	while(1) {
		jnx_thread_lock(&evt_lock);
		if(exiting) {

			jnx_thread_unlock(&evt_lock);
			return 0;
		}
		jnx_thread_unlock(&evt_lock);

		jnx_thread_lock(&queue_lock);
		event_object *e = jnx_queue_pop(event_queue);
		jnx_thread_unlock(&queue_lock);
		if(e) {
			jnx_event_update_subscribers(e);		
			JNX_MEM_FREE(e);
		}
		if(nanosleep(&tim , &tim2) < 0 )   
		{
			printf("Nano sleep system call failed \n");
			return NULL;
		}
	}
	return 0;	
}
void jnx_event_global_create(unsigned int max_queue) {
	srand(time(NULL));
	if(!subscription_list) {
		subscription_list = jnx_list_create();
	}
	if(!event_queue) {
		event_queue = jnx_queue_create();
	}
	jnx_thread_create_disposable(jnx_event_mainloop,NULL);
}
void jnx_event_global_destroy() {
	jnx_thread_lock(&evt_lock);
	exiting = 1;	
	jnx_thread_unlock(&evt_lock);
	jnx_list_destroy(&subscription_list);
	jnx_queue_destroy(&event_queue);
}
