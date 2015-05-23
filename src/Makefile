INC=../include
SRC=libcache.c

libcache: libcache.o
	 ar rcs libcache.a libcache.o
	@mv libcache.a ../lib
libcache.o: 
	 gcc -c ${SRC} -I${INC}
clean:
	rm libcache.a *.o
