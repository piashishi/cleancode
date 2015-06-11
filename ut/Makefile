UT_SRC= main.cc libpete.cc libpool_ut.cc libcache_test.cc libcache_ut.cc  hash_ut.cc list_ut.cc

ver=release

ifeq ($(ver), debug)
CFLAGS = -g -DDEBUG -fprofile-arcs -ftest-coverage -pg
else
CFLAGS = -O2 -pg
endif

BIT64=x86_64
ARCH:=$(shell uname -m)
ifeq ($(ARCH), $(BIT64))
LIB= ../lib -lUnitTest++_64  -lgcov
else
LIB= ../lib -lUnitTest++  -lgcov
endif


SRC = ../src/list.c \
      ../src/hash.c \
      ../src/libcache.c \
      ../src/libpool.c

#replace *.cc to *.o
UT_OBJ=$(UT_SRC:.cc=.o)
#replace *.c to *.o
SRC_OBJ=$(patsubst %.c,%.o,$(notdir $(SRC)))

INC+= -I../include \
      -I./include

.PHONY:clean

UT: $(SRC_OBJ) $(UT_OBJ)
	g++ $(CFLAGS) -o cache_ut *.o -L${LIB}
	@rm *.o

$(SRC_OBJ): 
	gcc $(CFLAGS) -c ${SRC} ${INC} 

$(UT_OBJ):
	g++  -D__STDC_FORMAT_MACROS $(CFLAGS) -c $(UT_SRC) ${INC}

clean:
	rm -f cache_ut
