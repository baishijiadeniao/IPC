CC := gcc
FLAG := -g -Wall -std=c++11
TARGET := pipe fifo socketpair shm tcp udp uds

all: ${TARGET}

pipe: pipe.cpp
	${CC} ${FLAG} -o $@ $<
fifo: fifo.cpp
	${CC} ${FLAG} -o $@ $<
socketpair: socketpair.cpp
	${CC} ${FLAG} -o $@ $<
shm: shm.cpp
	${CC} ${FLAG} -o $@ $<
tcp: tcp.cpp
	${CC} ${FLAG} -o $@ $<
udp: udp.cpp
	${CC} ${FLAG} -o $@ $<
uds: uds.cpp
	${CC} ${FLAG} -o $@ $<
.PHONY: test clean
test:
	sh ./test_run.sh
clean:
	rm ${TARGET}

