SAN_FLAGS += #-fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls #-fsanitize-memory-track-origins=2

EXTRA_CFLAGS=-O0 -g -Wall $(SAN_FLAGS)
EXTRA_EXEFLAGS=$(SAN_FLAGS)

LIBS += -L/usr/local/lib \
        -ldl -rdynamic -Ldstructs -ldstructs -Lmathexpr -lmathexpr -lm \
        -Llineread -llineread -lpcre2-8
CFLAGS += -Idstructs/src -Imathexpr/src -Ilineread/src \
          -I/usr/local/include \
          $(EXTRA_CFLAGS) \
          -DWITH_LIBRARIES

OBJECT = src/value.o \
         src/tcl.o src/std.o src/tclsh.o src/repl.o src/ext.o src/regexp.o

.PHONY: all clean test

all: dstructs mathexpr lineread tclsh #bindings_build

test: tclsh
	./tclsh -Itest/test.tcl test/*_test.tcl

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

tclsh: $(OBJECT)
	$(CC) $(EXTRA_EXEFLAGS) -o tclsh $(OBJECT) $(LIBS)

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

clean: dstructs_clean mathexpr_clean lineread_clean bindings_clean 
	-rm $(OBJECT)
	-rm doc/reference.html
	-rm tclsh
