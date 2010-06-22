OBJS=src/BTree.o src/Hash.o src/List.o src/SkipList.o src/Vector.o src/EString.o
LIBFILE=libdstructs.a
CFLAGS=-g

all: $(OBJS) $(LIBFILE)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBFILE):
	ar rc $(LIBFILE) $(OBJS)

clean:
	-rm $(OBJS) $(LIBFILE)

