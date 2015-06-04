INC=../include
SRC=libcache.c libpool.c list.c hash.c

libcache: libcache.o
	ar rcs libcache.a *.o
	@mv libcache.a ../lib
	@rm *.o
	@echo "Compile Done!"
libcache.o: 
	 gcc -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
	 -Wstrict-prototypes -Wmissing-prototypes -c ${SRC} -I${INC}
