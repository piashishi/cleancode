objects=libpete.o libcache_test.o

INC=../include
LIB= ../lib -lUnitTest++ -lcache

UT: $(objects)
	g++ -o cache_ut  ${objects}  -L${LIB}

$(objects): %.o: %.cc
	g++ -c $<  -o $@  -I${INC}
	 
clean:
	rm cache_ut *.o
