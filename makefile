.PHONY=clean
.DELETE_ON_ERROR=
$(CC)=gcc
CFLAGS:=--std=c99 -Wall
CFLAGDB:=--std=c99 -Wall -pg
parser: parser.o buffer.o asmtypes.o stable.o optable.o error.o parse_test.o
	$(CC) $^ -o $@

parserdb: parser.o buffer.o asmtypes.o stable.o optable.o error.o parse_test.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $^ $(CFLAGDB) -c

clean:
	rm -f *.o *.gch parser parserdb
