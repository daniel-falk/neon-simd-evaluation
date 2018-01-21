CC = gcc
AS = as
CFLAGS = -mfpu=neon -Wall -Werror

ASMFILES = fir_asm.o
CFILES = main.o fir_c.o fir_neon.o

BINARY = main

all: build run

build: ${ASMFILES} ${CFILES}
	${CC} ${CFLAGS} ${CFILES} ${ASMFILES} -o ${BINARY}

${CFILES} : %.o : %.c
	${CC} -c ${CFLAGS} $< -o $@

${ASMFILES} : %.o : %.s
	${AS} $< -o $@

run:
	./${BINARY} result

clean:
	rm -f *.o ${BINARY}
