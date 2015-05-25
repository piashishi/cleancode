INC=../include
SRC=libcache.c liblist.c

libcache: libcache.o
	ar rcs libcache.a libcache.o
	@mv libcache.a ../lib
	ar rcs liblist.a liblist.o
	@mv liblist.a ../lib
	
libcache.o: 
	gcc -c ${SRC} -I${INC}
	
clean:
	rm libcache.a *.o
	rm liblist.a *.o
