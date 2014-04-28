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
/**
 * @brief Event object is given to the use subscribing to an event
 */
typedef struct event_object {
	unsigned long identity;
	int evt_type;
	void *evt_data;
}event_object;
/**
 * @brief jnx_event_callback is used for event subscription
 */
typedef int (*jnx_event_callback)(event_object*);
/**
 * @brief the handle for an event
 */
typedef struct jnx_event_handle {
	unsigned long identity;
	int evt_type;
	jnx_event_callback c;
	int is_exiting;
}jnx_event_handle;


#define JNX_EVENT_SEND(X,Y) \
	jnx_event_send(jnx_event_object_create(X,Y)); 

#define JNX_EVENT_SUBSCRIBE(X,EVT_HANDLE,Y)\
	EVT_HANDLE = jnx_event_handle_create(X,Y); \
	jnx_event_subscribe(EVT_HANDLE); 

#define JNX_EVENT_UNSUBSCRIBE(EVT_HANDLE) \
	jnx_event_unsubscribe(EVT_HANDLE);
/**
 * @fn jnx_event_handle *jnx_event_handle_create(uint8_t *evt_type,jnx_event_callback c)
 * @brief Creates a handle for the event allowing the user to delete it
 * @param evt_type is the string to be hashed for the event to id
 * @return jnx_event_handle 
 */
jnx_event_handle *jnx_event_handle_create(uint8_t *evt_type,jnx_event_callback c);
/**
 * @fn jnx-event_subscribe(jnx_event_handle *e)
 * @brief internal function to setup subscription to events
 * @param e the event handler to setup subscription with
 */
void jnx_event_subscribe(jnx_event_handle *e);
/**
 * @fn jnx-event_unsubscribe(jnx_event_handle *e)
 * @brief internal function to setup unsubscription to events
 * @param e the event handler to setup unsubscription with
 */
void jnx_event_unsubscribe(jnx_event_handle *e);
/**
 *@fn event_object *jnx_event_object_create(uint8_t *evt_type, void *data)
 *@brief internal function to create an event object that is sent to event callbacks
 *@param evt_type uint8_t array name of event
 *@param data is any event data that may need to be transported
 */
event_object *jnx_event_object_create(uint8_t *evt_type,void *data);
/**
 * @fn jnx_event_send(event_object *e)
 * @brief internal function for sending an event object to subscribers
 * @param e event object for sending
 */
void jnx_event_send(event_object *e);
/**
 * @fn int jnx_event_is_of_type(uint8_t *evt_Type, event_object *e)
 * @brief will return 1 if event_type matches the event object_type
 * @evt_type is the targetted event type 
 * @e is the event_object to compare too
 * @return 1 on match 0 on failure
 */
int jnx_event_is_of_type(uint8_t *evt_type, event_object *e);
/**
 * @fn jnx_event_global_create()
 * @brief set up the event system and initial queues
 */
void jnx_event_global_create();
/**
 * @fn jnx_event_global_destroy()
 * @brief tears down internal data structures and releases event handlers
 */
void jnx_event_global_destroy();

#ifdef __cplusplus
	}
#endif
#endif
