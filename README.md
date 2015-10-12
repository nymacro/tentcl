# Tentcl
Tentcl is a small hand-parsed Tcl interpreter. This project was undertaken
to better my understanding of the parsing and interpretation of programming
languages. It is by no means complete, nor is it well-written -- it is more
of a toy than something useful.

For license details, see LICENSE

Aaron Marks
nymacro AT gmail DOT com

## BUILD REQUIREMENTS
In additional to the Tentcl source code, you will also need the `mathexpr`
source code, which must be placed in the Tentcl root directory. e.g.

```
  tentcl
  |- src
  |- mathexpr
  |- dstructs
  |- lineread
```

## BUILDING

```sh
cd tentcl
make all test
```

## LIMITATIONS
* No non-numerical comparisons for conditionals
* Everything

