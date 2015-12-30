UNAME := $(shell uname)

RTFLAGS=-lrt
ifeq ($(UNAME), Darwin)
RTFLAGS=-framework CoreServices
endif
OLEVEL=-O2 -DNDEBUG
CFLAGS=-Wall $(OLEVEL) -I libuv/include -std=gnu99
FILES=server.c
APP=server

all: $(FILES) libuv/libuv.a
	$(CC) $(CFLAGS) -o \
	$(APP) $(FILES) \
	libuv/libuv.a -lpthread -lcrypto -lm $(RTFLAGS)

libuv/libuv.a:
	$(MAKE) -C libuv

valgrind: OLEVEL=-O0 -g
valgrind: all
	valgrind --leak-check=full ./server

debug: OLEVEL=-O0 -g
debug: all

gprof: OLEVEL=-O0 -g -pg
gprof: all

clean:
	$(MAKE) -C libuv clean
	$(MAKE) -C http-parser clean
	rm -f server
	rm -rf *.dSYM
