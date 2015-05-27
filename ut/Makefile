UT_objects= main.o libpete.o libpool_ut.o libcache_test.o hash_ut.o list_ut.o
SRC_obj = list.o hash.o libcache.o libpool.o

SRC = ../src/list.c \
	  ../src/hash.c \
	  ../src/libcache.c \
	  ../src/libpool.c

INC=../include
LIB= ../lib -lUnitTest++  -lgcov

UT: $(SRC_obj) $(UT_objects)
	g++ -g -o cache_ut $(SRC_obj) $(UT_objects)  -L${LIB}
	@rm *.o
	@mv *.gcno cov

$(SRC_obj): 
	gcc -g -c ${SRC} -I${INC}  -fprofile-arcs -ftest-coverage

$(UT_objects): %.o: %.cc
	g++ -g -c $<  -o $@  -I${INC} -fprofile-arcs -ftest-coverage

clean:
	rm -f cache_ut
