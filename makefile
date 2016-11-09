.PHONY=clean
.DELETE_ON_ERROR=
$(CC)=gcc
CFLAGS=-O2 --std=c99 -Wall
parser: parser.o buffer.o asmtypes.o stable.o optable.o error.o parse_test.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $^ $(CFLAGS) -c

clean:
	rm -f *.o *.gch parser
