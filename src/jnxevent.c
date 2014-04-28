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
#include <jnxc_headers/jnxlog.h>
#include <jnxc_headers/jnxhash.h>
#include "jnxevent.h"
#include <time.h>
/* internal functions*/
typedef struct internal_datalink {
	jnx_event_subscriber *subscriber;
	jnx_event *event;
	jnx_event_system_handle *sys_handle;
}internal_datalink;

void *internal_async_send_subscriber(void *args);
void *internal_async_send_event(void *args);

#define _INTERNAL_SEND_EVENT_THREAD_DATA(X,Y) \
	internal_datalink *d = JNX_MEM_MALLOC(sizeof(internal_datalink)); \
d->subscriber = X; d->event = Y; \
jnx_thread_create_disposable(internal_async_send_subscriber,d);	
#define _INTERNAL_ADD_EVENT_THREAD_DATA(X,Y) \
	internal_datalink *d = JNX_MEM_MALLOC(sizeof(internal_datalink)); \
d->sys_handle = X; d->event = Y; \
jnx_thread_create_disposable(internal_async_send_event,d);
extern int jnx_hash_string(const char* input, int len);
int generate_identity() {
	return rand() % RAND_MAX;
}
void *internal_async_send_event(void *args) {
	internal_datalink *d = args;

	jnx_thread_lock(&d->sys_handle->queue_locker);
	jnx_queue_push(d->sys_handle->event_queue,d->event);
	jnx_thread_unlock(&d->sys_handle->queue_locker);
	return 0;
}
void internal_update_subscribers(jnx_event_system_handle *sys_handle,jnx_event *event) {

	jnx_node *head = sys_handle->subscription_list->head,
			 *reset = sys_handle->subscription_list->head;

	while(head) {
		jnx_event_subscriber *subscriber = head->_data;
		if(subscriber->evt_type == event->evt_type) {
			_INTERNAL_SEND_EVENT_THREAD_DATA(subscriber,event);	
		}	
		head = head->next_node;
	}
	head = reset;
}
void *internal_listen_loop(void *args) {
	jnx_event_system_handle *sys_handle = args;
	struct timespec tim,tim2;
	tim.tv_sec=0;
	tim.tv_nsec=2500000000L;	

	while(sys_handle->is_listening) {
		if(sys_handle->is_exiting) {
			sys_handle->is_listening = 0;
			return 0;
		}
		jnx_thread_lock(&sys_handle->subscription_locker);
		if(!sys_handle->subscription_list) {
			continue;
		}
		size_t sub_count = sys_handle->subscription_list->counter;
		jnx_thread_unlock(&sys_handle->subscription_locker);

		jnx_thread_lock(&sys_handle->queue_locker);
		if(!sys_handle->event_queue) {
			continue;
		}
		size_t queue_count = sys_handle->event_queue->list->counter;
		jnx_thread_unlock(&sys_handle->queue_locker);

		if(queue_count == 0 || sub_count == 0) {
			continue;
		}
		jnx_thread_lock(&sys_handle->queue_locker);
		jnx_event *evt = jnx_queue_pop(sys_handle->event_queue);
		jnx_thread_unlock(&sys_handle->queue_locker);
		if(evt) {
			jnx_thread_lock(&sys_handle->subscription_locker);
			internal_update_subscribers(sys_handle,evt);	
			jnx_thread_unlock(&sys_handle->subscription_locker);

		}
		nanosleep(&tim,&tim2);
	}
	JNX_LOGC(JLOG_DEBUG,"Started exiting...\n");

	jnx_list_destroy(&sys_handle->subscription_list);
	jnx_queue_destroy(&sys_handle->event_queue);
	JNX_MEM_FREE(sys_handle);
	return 0;
}
/*  end of internal functions */
void jnx_event_unsubscribe(jnx_event_system_handle *sys_handle, jnx_event_subscriber *subscriber) {
	jnx_list *temp = jnx_list_create();
	jnx_thread_lock(&sys_handle->subscription_locker);
	if(sys_handle->subscription_list->counter == 0) {
		jnx_thread_unlock(&sys_handle->subscription_locker);
		return;
	}
	jnx_node *head = sys_handle->subscription_list->head;
	while(head) {
		jnx_event_subscriber *inner_subscriber = head->_data;
		if(inner_subscriber->identity != subscriber->identity) {
			jnx_list_add(temp,inner_subscriber);
		}else {
			JNX_MEM_FREE(inner_subscriber);
		}
		head = head->next_node;
	}
	jnx_list_destroy(&sys_handle->subscription_list);
	sys_handle->subscription_list = NULL;
	sys_handle->subscription_list = temp;
	jnx_thread_unlock(&sys_handle->subscription_locker);
}
jnx_event_subscriber *jnx_event_subscribe(jnx_event_system_handle *sys_handle, char *evt_type,jnx_event_callback c) {
	jnx_event_subscriber *subscriber = JNX_MEM_MALLOC(sizeof(jnx_event_subscriber));
	subscriber->evt_type = jnx_hash_string(evt_type,strlen(evt_type));
	subscriber->identity = generate_identity();
	subscriber->callback = c;
	jnx_thread_lock(&sys_handle->subscription_locker);
	jnx_list_add(sys_handle->subscription_list,subscriber);
	jnx_thread_unlock(&sys_handle->subscription_locker);
	return subscriber;
}
void *internal_async_send_subscriber(void *args) {
	internal_datalink *d = args;
	d->subscriber->callback(d->event);
	return 0;
}

int jnx_event_send(jnx_event_system_handle *sys_handle, char *evt_type, void *evt_data) {

	jnx_event *e = JNX_MEM_MALLOC(sizeof(jnx_event));	
	e->evt_type = jnx_hash_string(evt_type,strlen(evt_type));
	e->evt_data = evt_data;
	e->identity = generate_identity();

	_INTERNAL_ADD_EVENT_THREAD_DATA(sys_handle,e);
	return 0;
}
void jnx_event_system_listen(jnx_event_system_handle *sys_handle) {
	sys_handle->is_listening = 1;
	jnx_thread_create_disposable(internal_listen_loop,sys_handle);
}
jnx_event_system_handle* jnx_event_system_create() {

	jnx_event_system_handle *sys_handle = JNX_MEM_MALLOC(sizeof(jnx_event_system_handle));
	sys_handle->is_exiting = 0;
	sys_handle->is_listening = 1;
	sys_handle->subscription_list = jnx_list_create();
	sys_handle->event_queue = jnx_queue_create();
	return sys_handle;	
}
void jnx_event_system_destroy(jnx_event_system_handle **sys_handle) {
	jnx_event_system_handle *sys = *sys_handle;
	sys->is_exiting = 1;
}
