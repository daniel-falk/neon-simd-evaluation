CC = gcc
CFLAGS = -mfpu=neon -Wall -Werror

FILES = main.c fir_c.c fir_neon.c

BINARY = main

all: build run

build:
	${CC} ${CFLAGS} ${FILES} -o ${BINARY}

run:
	./${BINARY} result
