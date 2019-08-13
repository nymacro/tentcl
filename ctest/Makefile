.PHONY: all clean test

CFLAGS=$(EXTRA_CFLAGS) -I../src -I../dstructs/src
LDFLAGS=$(EXTRA_LDFLAGS) -L../ -ltcl -L../dstructs -ldstructs

TESTS = list_test

all: $(TESTS)

test: all
	./list_test

list_test: list_test.o
	$(CC) -o $@ list_test.o $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	-rm $(TESTS)
	-rm *.o
