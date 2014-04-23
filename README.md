eventHandler
============

C based event handler system

Usage
===========

```c
JNX_EVENT_SUBSCRIBE("BLAH!",callback_function);
//And perhaps in another thread or another time in the program...
JNX_EVENT_SEND("BLAH!","DATA");
```

Dependency
============

https://github.com/AlexsJones/jnxlibc
Install this first :) 


Build
===========

make
