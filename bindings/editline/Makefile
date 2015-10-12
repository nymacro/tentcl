OBJS=editline.o
LIB=editline.so
CFLAGS=-I../../dstructs/src

all: $(LIB)

$(LIB): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)

.c.o:
	$(CC) -Wall -fPIC -c -o $@ $< $(CFLAGS)

clean:
	-rm $(OBJS) $(LIB)

