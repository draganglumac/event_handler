subs:
	git submodule init
	git submodule update
	sh -c "cd jnxlibc && ./setup/build_and_install && cd ../"
all: subs
	gcc *.c -o test -ljnxc -DJNX_MEMORY_MANAGEMENT
	
debug:	
	gcc *.c -o test -ljnxc -DJNX_MEMORY_MANAGEMENT
