.PHONY=clean
.DELETE_ON_ERROR=
$(CC)=gcc
CFLAGS=-O2 --std=c99 -Wall
parser: parser.o buffer.o asmtypes.o opcodes.o stable.o optable.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $^ $(CFLAGS) -c

clean:
	rm -f *.o *.gch parser
