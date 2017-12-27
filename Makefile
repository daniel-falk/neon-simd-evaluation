CC = gcc
CFLAGS = -mfpu=neon

FILES = main.c

BINARY = main

all: build run

build:
	${CC} ${CFLAGS} ${FILES} -o ${BINARY}

run:
	./${BINARY} result
