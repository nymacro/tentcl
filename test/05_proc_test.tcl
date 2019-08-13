set did_override 0
proc count_args {args} {
    llength $args
}
test "varargs" { assert {[count_args 1 2 3] == 3} }

proc shadow_func {} {
    proc shadow_func {} {
        return 1
    }
    shadow_func
}
test "proc_name_shadow" { assert {[shadow_func] == 1} }

proc fact {n} {
    if {$n < 2} {
        return $n
    } else {
        return [expr [fact [incr $n -1]] * $n]
    }
}
test "proc_recursion" {
    set f [fact 5]
    assert "$f == 120"
}