OBJS=src/BTree.o src/Hash.o src/List.o src/SkipList.o src/Vector.o src/EString.o
LIBFILE=libdstructs.a
CFLAGS=$(EXTRA_CFLAGS)

.PHONY: all test clean

all: $(OBJS) $(LIBFILE)

test: all
	make -C test all test

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBFILE):
	ar rc $(LIBFILE) $(OBJS)

clean:
	-rm $(OBJS) $(LIBFILE)
	-make -C test clean

