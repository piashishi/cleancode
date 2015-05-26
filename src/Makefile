INC=../include
SRC=libcache.c pool.c list.c hash.c

libcache: libcache.o
	ar rcs libcache.a *.o
	@mv libcache.a ../lib
libcache.o: 
	 gcc -c ${SRC} -I${INC}
clean:
	rm libcache.a *.o
