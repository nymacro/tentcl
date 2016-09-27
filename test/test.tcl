proc eval_catch {test_body} {
    set err [catch {eval $test_body}]
    #repl
    if {$err != 0} {
        puts stderr "    Fail ($err)"
    }
}

proc eval_catch_pending {test_body} {
    set err [catch { eval $test_body }]
    if {$err != 0} {
        puts stderr "    Fail ($err)"
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
        puts "    failed condition $condition"
        fail
    }
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
