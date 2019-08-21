# Tentcl
Tentcl is a small hand-parsed Tcl interpreter. This project was undertaken
to better my understanding of the parsing and interpretation of programming
languages.

It is an old project which was started in 2006. It is by no means complete;
nor is it well-written. It is extremely inefficient in its execution, uses
only lexical variables, and does not support much of the syntax of the Tcl
l anguage.

For license details, see LICENSE

Aaron Marks
nymacro *AT* gmail *DOT* com

## BUILD REQUIREMENTS
In additional to the Tentcl source code, you will also need the `mathexpr`
source code, which must be placed in the Tentcl root directory. e.g.

```
  tentcl
  |- src/
  |- mathexpr/
  |- dstructs/
  |- lineread/
  |- Makefile
```

## BUILDING
To build the `tclsh` program, run the following. Run `./tclsh` to run a Tcl REPL, or
`./tclsh --help` for other execution options.

```sh
cd tentcl
make all
```

## TESTING
```sh
make test
```

## LIMITATIONS
* Terrible runtime.
* Terrible spaghetti-code parsing.
* No non-numerical comparisons for conditionals (`expr` uses `mathexpr`, which does not
  support non-`int` types)
* Inefficient.
* Not thoroughly tested.
* Other things.
