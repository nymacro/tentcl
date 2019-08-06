set did_override 0

proc count_args {args} {
    llength $args
}

test "varargs" { assert {[count_args 1 2 3] == 3} }
