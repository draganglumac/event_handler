BUILDIR=src
subs:
	git submodule init
	git submodule update
	sh -c "cd jnxlibc && ./setup/build_and_install && cd ../"
all: subs
	gcc ${BUILDIR}/*.c *.c -o test -ljnxc -DJNX_MEMORY_MANAGEMENT
	
debug:	
	gcc ${BUILDIR}/*.c *.c -o test -ljnxc -DJNX_MEMORY_MANAGEMENT
