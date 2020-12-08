CC = gcc

OPTIONS = -Wall -Wextra -g -pthread $(shell xml2-config --cflags)
LINKERFLAG = -lncursesw -lm $(shell xml2-config --libs)

FILES = $(wildcard *.c)

colormyday: ${FILES} colormyday.h
	@${CC} ${OPTIONS} -o colormyday ${FILES} ${LINKERFLAG} 

BINARIES := /usr/local
install: colormyday
	install -d ${BINARIES}/bin/
	install -m 755 colormyday ${BINARIES}/bin/




