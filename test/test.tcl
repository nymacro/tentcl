set tests_pass 0
set tests_fail 0
set tests_pending 0

proc incr_fail {} {
    upvar 0 tests_fail tests_fail
    set tests_fail [incr $tests_fail]
}
proc incr_pass {} {
    upvar 0 tests_pass tests_pass
    set tests_pass [incr $tests_pass]
}
proc incr_pending {} {
    upvar 0 tests_pending tests_pending
    set tests_pending [incr $tests_pending]
}

proc eval_catch {test_body} {
    set err [catch {eval $test_body}]
    if {$err != 0} {
        puts stderr "    Fail ($err)"
        incr_fail
    } else {
        incr_pass
    }
}

proc eval_catch_pending {test_body} {
    set err [catch { eval $test_body }]
    if {$err != 0} {
        puts stderr "    Fail ($err)"
    } else {
        incr_pending
    }
}

proc test {test_name test_body} {
    puts "t $test_name"
    eval_catch $test_body
}

proc pending {test_name test_body} {
    puts "p $test_name"
    eval_catch_pending $test_body
}

proc assert {condition} {
    if $condition {
        return 1
    } else {
        # repl
        puts "    failed condition $condition"
        fail
    }
}

proc assert_error {error block} {
    set err [catch $block]
    assert "$err == $error"
}

proc fail {} {
    exit 2
}

proc run {} {
    set test_files [glob "test/0*_*.tcl"]
    foreach test $test_files {
        puts "$test"
        set ret [catch { source $test }]
        if {$ret != 0} { puts stderr "  ERROR running $test ($ret)" }
    }
    puts "[llength $test_files] suites"
}
