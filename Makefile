FILES = *.c
INCLUDES = -I.
CFLAGS = -std=c99 -g
OUTFILE = compiler

main: ${FILES}
	${CC} ${CFLAGS} ${INCLUDES} -o ${OUTFILE} ${FILES}
