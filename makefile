.PHONY=clean
.DELETE_ON_ERROR=
$(CC)=gcc
CFLAGS:=--std=c99 -Wall
CFLAGDB:=--std=c99 -Wall -g

macas: macas.o asm.o error.o parser.o buffer.o stable.o asmtypes.o optable.o
	$(CC) $^ -o $@
	rm -f *.o *.gch

maclk: maclk.o taglist.o buffer.o error.o
	$(CC) $^ -o $@
	rm -f *.o *.gch

%.o: %.c
	$(CC) $^ $(CFLAGDB) -c

clean:
	rm -f *.o *.gch parser parserdb macas maclk
