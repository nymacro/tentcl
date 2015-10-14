.PHONY: all clean

all: basic_lib sqlite3_lib
	echo "done building bindings"

clean: basic_clean sqlite3_clean
	echo "cleaning up bindings"

basic_lib:
	$(MAKE) -C basic

basic_clean:
	$(MAKE) -C basic clean

sqlite3_lib:
	$(MAKE) -C sqlite3

sqlite3_clean:
	$(MAKE) -C sqlite3 clean

editline_lib:
	$(MAKE) -C editline

editline_clean:
	$(MAKE) -C editline clean

