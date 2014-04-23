/*
 * =====================================================================================
 *
 *       Filename:  eventhandler.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/23/14 08:18:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jonesax (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__
#include <stdint.h>
#include <jnxc_headers/jnxmem.h>
typedef struct event_object {
	unsigned long identity;
	uint8_t *evt_type;
	void *evt_data;
}event_object;

typedef int (*jnx_event_callback)(event_object*);

typedef struct jnx_event {
	unsigned long identity;
	uint8_t *evt_type;
	jnx_event_callback c;
}jnx_event;
#define JNX_EVENT_SEND(X,Y) \
	jnx_event_send(jnx_event_object_create(X,Y)); 
#define JNX_EVENT_SUBSCRIBE(X,Y)\
	jnx_event_callback c = &Y; \
	jnx_event *h = jnx_event_handle_create(X,c); \
	jnx_event_subscribe(h); ///free??

#define JNX_EVENT_UNSUBSCRIBE

jnx_event *jnx_event_handle_create(uint8_t *evt_type,jnx_event_callback c);

void jnx_event_subscribe(jnx_event *e);

event_object *jnx_event_object_create(uint8_t *evt_type,void *data);

int jnx_event_send(event_object *e);

void jnx_event_global_create(unsigned int max_queue);

void jnx_event_global_destroy();
#endif
