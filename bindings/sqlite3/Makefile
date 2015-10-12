OBJS=sqlite3.o
LIBS=-lsqlite3
LIB=sqlite3.so
CFLAGS=-I../../dstructs/src

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS) $(LIBS)

.c.o:
	$(CC) -Wall -fPIC -c -o $@ $< $(CFLAGS)

clean:
	-rm $(OBJS) $(LIB)

