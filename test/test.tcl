proc test {test_name test_body} {
    puts "  $test_name"
    set ret [catch {eval $test_body}]
    if {$ret != 0} { puts stderr "    failed ($ret)" }
}

proc pending {test_name test_body} {
    puts "  PENDING: $test_name"
    set ret [catch {eval $test_body}]
    if {$ret != 0} {puts stderr "    passed unexpectedly"}
}

proc assert {condition} {
    if {$condition} { } else { puts "    failed condition $condition"; fail }
}

proc fail {} {
    puts stderr "    failed test"
    exit 1
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

