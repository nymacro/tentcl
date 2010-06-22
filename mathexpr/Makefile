OBJS=src/mathexpr.o
LIBFILE=libmathexpr.a
CFLAGS=-g -I../dstructs/src

all: $(LIBFILE)

test: clean all
	$(CC) -o test test.c -L. -lmathexpr -lm

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBFILE): $(OBJS)
	ar rc $(LIBFILE) $(OBJS)

clean:
	-rm $(OBJS) $(LIBFILE) test

