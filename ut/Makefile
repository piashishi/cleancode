objects= main.o libpete.o libcache_test.o hash_ut.o

INC=../include
LIB= ../lib -lUnitTest++ -lcache

UT: $(objects)
	g++ -g -o cache_ut  ${objects}  -L${LIB}

$(objects): %.o: %.cc
	g++ -g -c $<  -o $@  -I${INC}
	 
clean:
	rm cache_ut *.o
