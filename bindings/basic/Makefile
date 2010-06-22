OBJS=basic.o
LIB=basic.so
CFLAGS=-I../../dstructs/src

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared -Wl -o $(LIB) $(OBJS)

.c.o:
	$(CC) -Wall -fPIC -c -o $@ $< $(CFLAGS)

clean:
	-rm $(OBJS) $(LIB)

