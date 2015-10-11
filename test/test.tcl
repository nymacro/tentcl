proc test {test_name test_body} {
    puts "  $test_name"
    eval $test_body
}

proc assert {condition} {
    if {$condition} { } else { puts "failed condition $condition"; exit 1 }
}

proc fail {} {
    puts stderr "failed test"
    exit 1
}

proc run {} {
    set test_files [glob "test/0*.tcl"]
    foreach test $test_files {
        puts "running $test"
        source $test
    }
    puts "OK [llength $test_files] suites"
}

run

