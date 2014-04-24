BUILDIR=src
FLAGS=-Wall -fPIC -c
subs:
	git submodule init
	git submodule update
	sh -c "cd jnxlibc && ./setup/build_and_install && cd ../"

all: library
	sudo mv libjnxevent.so.1.0 /usr/lib
	sudo cp src/*.h /usr/include	
	sudo ln -sf /usr/lib/libjnxevent.so.1.0 /usr/lib/libjnxevent.so
	sudo ln -sf /usr/lib/libjnxevent.so.1.0 /usr/lib/libjnxevent.so.1
test: debug
	sh -c "./test"
debug:	
	gcc ${BUILDIR}/*.c *.c -o test -ljnxc -DJNX_MEMORY_MANAGEMENT

library:
	gcc ${FLAGS} $(BUILDIR)/*.c
	gcc -shared -Wl,-soname,libjnxevent.so -o libjnxevent.so.1.0 -ldl -pthread *.o
	rm *.o
