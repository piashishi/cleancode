INC=../include
SRC=libcache.c libpool.c list.c hash.c

libcache: libcache.o
	ar rcs libcache.a *.o
	@mv libcache.a ../lib
	@rm *.o
	@echo "Compile Done!"
libcache.o: 
	 gcc -c ${SRC} -I${INC}
