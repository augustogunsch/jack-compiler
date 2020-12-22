FILES = *.c */*.c
INCLUDES = -I. -I./parser/ -I./compiler -I./vm -I./tokenizer
CFLAGS = -std=c99 -g
OUTFILE = jack-compiler

main: ${FILES}
	${CC} ${CFLAGS} ${INCLUDES} -o ${OUTFILE} ${FILES}
