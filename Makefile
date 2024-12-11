CC = gcc
CFLAGS = -I

msh:
	${CC} ${CFLAGS} ./include/ -o ./bin/msh src/*.c

clean:
	rm -f ./bin/msh ./tests/test_parse_tok ./tests/test_separate_args ./tests/test_history
