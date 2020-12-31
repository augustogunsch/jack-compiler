FILES = *.c */*.c
LIBRARIES = -lpthread
INCLUDES = -I. -I./parser/ -I./compiler/ -I./vm/ -I./tokenizer/ -I./misc/
CFLAGS = -std=c99 -g
OUTFILE = jack-compiler

main: ${FILES}
	${CC} ${CFLAGS} ${LIBRARIES} ${INCLUDES} -o ${OUTFILE} ${FILES}
