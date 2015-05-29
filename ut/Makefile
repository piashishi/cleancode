UT_SRC= main.cc libpete.cc  libcache_test.cc hash_ut.cc list_ut.cc
#UT_SRC= main.cc libpete.cc libpool_ut.cc libcache_test.cc hash_ut.cc list_ut.cc

SRC = ../src/list.c \
      ../src/hash.c \
      ../src/libcache.c \
#      ../src/libpool.c

#replace *.cc to *.o
UT_OBJ=$(UT_SRC:.cc=.o)
#replace *.c to *.o
SRC_OBJ=$(patsubst %.c,%.o,$(notdir $(SRC)))

INC=../include
LIB= ../lib -lUnitTest++  -lgcov


.PHONY:clean

UT: $(SRC_OBJ) $(UT_OBJ)
	g++ -g -o cache_ut *.o -L${LIB}
	@rm *.o
	@mv *.gcno cov

$(SRC_OBJ): 
	gcc -g -c ${SRC} -I${INC}  -fprofile-arcs -ftest-coverage

$(UT_OBJ):
	g++ -g -c $(UT_SRC) -I${INC} -fprofile-arcs -ftest-coverage

clean:
	rm -f cache_ut
