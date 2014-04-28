/** @file jnxevent.h
 *  @brief Jnx event system implementation
 *
 *  #include <jnxc_headers/jnxevent.h>
 */
#ifndef __JNX_EVENT_H__
#define __JNX_EVENT_H__
#ifdef __cplusplus
	extern "C" {
#endif
#include <stdint.h>
#include <jnxc_headers/jnxmem.h>
#include <jnxc_headers/jnxqueue.h>
/**
 * @brief Event object is given to the use subscribing to an event
 */

		
typedef struct jnx_event {
	unsigned long identity;
	int evt_type;
	void *evt_data;
}jnx_event;

typedef void (*jnx_event_callback)(jnx_event*);

typedef struct jnx_event_subscriber {
	unsigned long identity;
	int evt_type;
	jnx_event_callback callback;
}jnx_event_subscriber;

typedef struct jnx_event_system_handle {
	int is_listening;
	int is_exiting;
	
	jnx_list *subscription_list;
	jnx_queue *event_queue;

	jnx_thread_mutex subscription_locker;
	jnx_thread_mutex queue_locker;
}jnx_event_system_handle;

/*  firing */
int jnx_event_send(jnx_event_system_handle *sys_handle, char *evt_type, void *evt_data);

/* subscription */
jnx_event_subscriber *jnx_event_subscribe(jnx_event_system_handle *sys_handle, char *evt_type,jnx_event_callback c);

void jnx_event_system_listen(jnx_event_system_handle *e);

jnx_event_system_handle* jnx_event_system_create();

void jnx_event_system_destroy(jnx_event_system_handle **e);


#ifdef __cplusplus
	}
#endif
#endif
