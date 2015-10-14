OBJS=src/LineRead.o
LIBFILE=liblineread.a
CFLAGS= -I../dstructs/src

.PHONY: all clean test

all: $(LIBFILE)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBFILE): $(OBJS)
	ar rc $(LIBFILE) $(OBJS)

test: $(LIBFILE)
	$(CC) -o test test.c -L. -llineread -Isrc $(CFLAGS) -L../dstructs -ldstructs

clean:
	-rm $(OBJS) $(LIBFILE) test

