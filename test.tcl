# source lib/expr.tcl

source test/test_lib.tcl
foreach f [glob test/0*_test.tcl] {
    puts "$f:"
    source $f
    puts
}
source test/test_summary.tcl
