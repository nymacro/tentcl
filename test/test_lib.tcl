set tests_pass 0
set tests_fail 0
set tests_pending 0
set tests_skipped 0

proc incr_fail {} {
    upvar 0 tests_fail tests_fail
    set tests_fail [incr tests_fail]
}
proc incr_pass {} {
    upvar 0 tests_pass tests_pass
    set tests_pass [incr tests_pass]
}
proc incr_pending {} {
    upvar 0 tests_pending tests_pending
    set tests_pending [incr tests_pending]
}
proc incr_skipped {} {
    upvar 0 tests_skipped tests_skipped
    set tests_skipped [incr tests_skipped]
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
    set err_val [catch { eval $test_body } err]
    if {$err != 0} {
        puts stderr "    Fail ($err : $err_val)"
    } else {
        incr_pending
    }
}

proc test {test_name test_body} {
    upvar args args
    if {[match $args $test_name]} {
	puts "t $test_name"
	eval_catch $test_body
    } else {
	puts "s $test_name"
	incr_skipped
    }
}

proc pending {test_name test_body} {
    puts "p $test_name"
    eval_catch_pending $test_body
}

proc skip {test_name test_body} {
    puts "x $test_name"
    incr_skipped
}

proc assert {condition} {
    uplevel 1 {
        if $condition {
            noop
        } else {
            puts "    failed condition $condition"
            repl
            fail
        }
    }
}

proc assert_error {error block} {
    set err [catch $block ret]
    assert {$err == $error}
}

proc fail {args} {
    if {[llength $args] > 0} {
        throw [lindex $args 0]
    } else {
        throw
    }
}
