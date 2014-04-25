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
#include <jnxc_headers/jnxhash.h>
#include "jnxevent.h"
#include <time.h>
jnx_thread_mutex evt_lock;
jnx_thread_mutex queue_lock;
pthread_t evt_thr;
pthread_mutex_t event_lock;
static int exiting = 0;
static jnx_list *subscription_list = NULL;
static jnx_queue *event_queue = NULL;

extern int jnx_hash_string(const char* input, int map_size);

unsigned long jnx_event_identity_create() {
	return rand();
}
int jnx_event_is_of_type(uint8_t *evt_type, event_object *e) {

	int evt = jnx_hash_string(evt_type,strlen(evt_type));
	if(e->evt_type == evt) {
		return 1;
	}
	return 0;
}
jnx_event_handle *jnx_event_handle_create(uint8_t *evt_type,jnx_event_callback c) {
	assert(evt_type);
	assert(c);
	jnx_event_handle *e = JNX_MEM_MALLOC(sizeof(jnx_event_handle));
	e->c = c;
	e->evt_type = jnx_hash_string(evt_type,strlen(evt_type));
	e->identity = jnx_event_identity_create();	
	JNX_LOGC(JLOG_NORMAL,"Generated eventhandler with ID:%ld\n",e->identity);
	return e;
}
event_object *jnx_event_object_create(uint8_t *evt_type,void *data) {
	JNX_LOGC(JLOG_NORMAL,"Creating event object\n");
	event_object *eo = JNX_MEM_MALLOC(sizeof(event_object));
	eo->evt_type = jnx_hash_string(evt_type,strlen(evt_type));
	eo->evt_data = data;
	eo->identity = jnx_event_identity_create();
	JNX_LOGC(JLOG_NORMAL,"Generated event with ID:%lu\n",eo->identity);
	return eo;
}
void jnx_event_object_destroy(event_object *e) {
	JNX_MEM_FREE(e);
}
void jnx_event_handle_destroy(jnx_event_handle *e) {
	JNX_MEM_FREE(e);
}
void jnx_event_unsubscribe(jnx_event_handle *e) {
	jnx_list *temp = jnx_list_create();
	jnx_thread_lock(&evt_lock);
	jnx_node *head = subscription_list->head;
	while(head) {
		jnx_event_handle *je = head->_data;
		if(je->identity != e->identity) {
			jnx_list_add(temp,je);
		}
		head = head->next_node;
	}
	jnx_list_destroy(&subscription_list);
	subscription_list = temp;
	JNX_LOGC(JLOG_NORMAL,"Number of events %d\n",subscription_list->counter);
	jnx_thread_unlock(&evt_lock);
	jnx_event_handle_destroy(e);
}
void jnx_event_subscribe(jnx_event_handle *e) {
	JNX_LOGC(JLOG_NORMAL,"Subscribing new handle [%lu:%d]\n",e->identity,e->evt_type);
	jnx_thread_lock(&evt_lock);
	jnx_list_add(subscription_list,e);
	jnx_thread_unlock(&evt_lock);
}
typedef struct datalink {
	event_object *e;
	jnx_event_handle *h;
}datalink;
void *async_update(void *args) {
	datalink *d = args;
	d->h->c(d->e);
	JNX_MEM_FREE(d);
}
void jnx_event_update_subscribers(event_object *e) {
	assert(e);	
	jnx_thread_lock(&evt_lock);
	jnx_node *head = subscription_list->head,
			 *reset = subscription_list->head;
	while(head) {
		jnx_event_handle *je = head->_data;
		if(je->evt_type == e->evt_type) {
			datalink *d = JNX_MEM_MALLOC(sizeof(datalink));
			d->h = je;
			d->e = e;
			jnx_thread_create_disposable(async_update,d);	

		}
		head = head->next_node;
	}
	head = reset;
	jnx_thread_unlock(&evt_lock);
	JNX_LOGC(JLOG_NORMAL,"Queue length %d\n",event_queue->list->counter);
}
void jnx_event_send(event_object *e) {
	jnx_thread_lock(&queue_lock);
	jnx_queue_push(event_queue,e);
	jnx_thread_unlock(&queue_lock);
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
void jnx_event_global_create() {
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

	jnx_node *head = subscription_list->head;
	while(head) {
		jnx_event_handle *e = head->_data;
		jnx_event_handle_destroy(e);
		head = head->next_node;
	}
	jnx_list_destroy(&subscription_list);

	event_object *eo;
	while((eo = jnx_queue_pop(event_queue)) != NULL) {
		jnx_event_object_destroy(eo);
	}	
	jnx_queue_destroy(&event_queue);
}
