SAN_FLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls #-fsanitize-memory-track-origins=2

EXTRA_CFLAGS=-O0 -g -Wall $(SAN_FLAGS)
EXTRA_LDFLAGS=$(SAN_FLAGS)

LDFLAGS += -L/usr/local/lib \
           -ldl -rdynamic -Ldstructs -ldstructs -Lmathexpr -lmathexpr -lm \
           -Llineread -llineread -lpcre2-8

CFLAGS = -Idstructs/src -Imathexpr/src -Ilineread/src \
         -I/usr/local/include \
         $(EXTRA_CFLAGS) \
         -DWITH_LIBRARIES

OBJECT = src/value.o \
         src/tcl.o src/std.o src/repl.o src/ext.o src/regexp.o

TCLSH_OBJS = src/tclsh.o

LIBS = libtcl.a dstructs/libdstructs.a mathexpr/libmathexpr.a lineread/liblineread.a

.PHONY: all clean test ctest

all: dstructs mathexpr lineread tclsh #bindings_build

test: tclsh ctest
	./tclsh -Itest/test.tcl test/*_test.tcl

ctest: all
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" EXTRA_LDFLAGS="$(EXTRA_LDFLAGS)" -C ctest all test

ctest_clean:
	$(MAKE) -C ctest clean

dstructs: dstructs/libdstructs.a

dstructs/libdstructs.a:
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -C dstructs

dstructs_clean:
	-$(MAKE) -C dstructs clean

mathexpr: mathexpr/libmathexpr.a

mathexpr/libmathexpr.a:
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -C mathexpr

mathexpr_clean:
	-$(MAKE) -C mathexpr clean

lineread: lineread/liblineread.a

lineread/liblineread.a:
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -C lineread

lineread_clean:
	-$(MAKE) -C lineread clean

bindings_build:
	$(MAKE) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" -C bindings

bindings_clean:
	-$(MAKE) -C bindings clean

libtcl.a: $(OBJECT)
	$(AR) rc libtcl.a $(OBJECT)

tclsh: $(LIBS) $(TCLSH_OBJS)
	$(CC) $(EXTRA_LDFLAGS) -o tclsh $(TCLSH_OBJS) -L. -ltcl $(LDFLAGS)

.c.o:
	$(CC) -o $@ -c $< $(CFLAGS)

docs: docgen.pl src/std.c doc_reference

doc_reference:
	perl docgen.pl src/std.c > doc/reference.html

install: all
	mkdir -p $(DESTDIR)/usr/bin
	cp tclsh $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/doc/tentcl
	cp docs.html $(DESTDIR)/usr/share/doc/tentcl

clean: dstructs_clean mathexpr_clean lineread_clean bindings_clean ctest_clean
	-rm $(OBJECT)
	-rm doc/reference.html
	-rm tclsh
