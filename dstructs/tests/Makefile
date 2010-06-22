CFLAGS+= -Wall -I../src
LIBS+= -L.. -ldstructs

all: list btree hash

list: list.o
	$(CC) -o list list.o $(LIBS)

btree: btree.o
	$(CC) -o btree btree.o $(LIBS)

hash: hash.o
	$(CC) -o hash hash.o $(LIBS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	-rm *.o

