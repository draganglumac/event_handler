event_handler
============

C based event handler system

Usage
===========

```c
jnx_event_handler *handle;
JNX_EVENT_SUBSCRIBE("BLAH!",handle,callback_function);
//And perhaps in another thread or another time in the program...
JNX_EVENT_SEND("BLAH!","DATA");
//And even later on
JNX_EVENT_UNSUBSCRIBE(handle);
```
```
| 						Thread A 					    |             Thread B  		     |
| ----------------------------------------------------- | ---------------------------------- |
| JNX_EVENT_SUBSCRIBE("Blah!",handle,callback_function);|                   			     |
| 														|                    				 |
|                                                       | JNX_EVENT_SEND("Blah!",&somedata); |
|   													| 									 |
| JNX_EVENT_UNSUBSRIBE(handle);                         |                                    |

```




Build
===========

Will install submodules and pull deps automagically

make subs
make test
make all  //Will prompt for sudo password to install


Usage & Linking
===========

In the makefile...

```
gcc *.c -ljnxevent
```

In the code 

```
#include <jnxevent.h>
```
