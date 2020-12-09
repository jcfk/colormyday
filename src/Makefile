CC = gcc

OPTIONS = -Wall -Wextra -g -std=c11 -pedantic -Wmissing-prototypes \
	-Wstrict-prototypes -pthread \
	$(shell xml2-config --cflags) -Wshadow -Wpointer-arith \
	-Wcast-qual
LINKERFLAG = -lncursesw -lm $(shell xml2-config --libs)

FILES = $(wildcard *.c)

colormyday: ${FILES} colormyday.h
	@${CC} ${OPTIONS} -o colormyday ${FILES} ${LINKERFLAG} 

BINARIES := /usr/local
install: colormyday
	install -d ${BINARIES}/bin/
	install -m 755 colormyday ${BINARIES}/bin/




