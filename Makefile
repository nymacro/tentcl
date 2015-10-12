#CC=gcc
LIBS+= -ldl -rdynamic -Ldstructs -ldstructs -Lmathexpr -lmathexpr -lm \
       -Llineread -llineread
CFLAGS+= -Idstructs/src -Imathexpr/src -Ilineread/src \
         -g -Wall \
         -DWITH_LIBRARIES
OBJECT= src/value.o \
        src/tcl.o src/std.o src/ext.o src/tclsh.o

#LEAK_CHECK=1

ifdef LEAK_CHECK
CFLAGS+= -DLEAK_CHECK -Ibdwgc/include
LIBS+= -Lbdwgc/.libs -lgc
endif

all: dstructs mathexpr lineread tclsh #bindings_build

test: tclsh
	./tclsh test/test.tcl

dstructs: dstructs/libdstructs.a

dstructs/libdstructs.a:
	make -C dstructs

dstructs_clean:
	-make -C dstructs clean

mathexpr: mathexpr/libmathexpr.a

mathexpr/libmathexpr.a:
	make -C mathexpr

mathexpr_clean:
	-make -C mathexpr clean

lineread: lineread/liblineread.a

lineread/liblineread.a:
	make -C lineread

lineread_clean:
	-make -C lineread clean

bindings_build:
	make -C bindings

bindings_clean:
	-make -C bindings clean

tclsh: $(OBJECT)
	$(CC) -o tclsh $(OBJECT) $(LIBS)

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

