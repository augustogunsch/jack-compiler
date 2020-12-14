FILES = tokenizer.c main.c parser.c printer.c
INCLUDES = -I.
CFLAGS = -std=c99 -g
OUTFILE = compiler

main: ${FILES}
	${CC} ${CFLAGS} ${INCLUDES} -o ${OUTFILE} ${FILES}
