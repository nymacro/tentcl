# Tentcl
Tentcl is a small hand-parsed Tcl interpreter. This project was undertaken
to better my understanding of the parsing and interpretation of programming
languages.

It is an old project which was started in 2006. It is by no means complete,
nor is it well-written -- it is more of a toy than something useful. It is
extremely and inefficient in its execution, uses only lexical variables, and
does not support much of the syntax of the Tcl language.

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

## LIMITATIONS
* No non-numerical comparisons for conditionals (`expr` uses `mathexpr`, which does not
  support non-`int` types)
* Inefficient.
* Not thoroughly tested.
* Other things.
