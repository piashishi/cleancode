INC=../include
SRC=libcache.c libpool.c list.c hash.c

ver=release

ifeq ($(ver), debug)
CFLAGS= -std=c99  -Wall -Wshadow -Wpointer-arith -Wcast-qual \
	 -Wstrict-prototypes -Wmissing-prototypes -DDEBUG -c
else
CFLAGS= -std=c99  -Wall -Wshadow -Wpointer-arith -Wcast-qual \
	 -Wstrict-prototypes -Wmissing-prototypes -c
endif

         

libcache: libcache.o
	ar rcs libcache.a *.o
	@mv libcache.a ../lib
	@rm *.o
	@echo "Compile Done!"
libcache.o: 
	 gcc $(CFLAGS) -c ${SRC} -I${INC}
