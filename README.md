eventHandler
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

Dependency
============

https://github.com/AlexsJones/jnxlibc
Install this first :) 


Build
===========

make
