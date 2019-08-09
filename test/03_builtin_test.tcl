pending "list" { assert {[list 1 2 3] == {1 2 3}} }
pending "listne" { assert {[list 1 2 3] != {1 2 3}} }
test "llength" { assert {[llength {1 2 3}] == 3} }
test "list2"   { assert {[llength [list 1 2 3]] == 3 } }
test "lindex"  { assert {[lindex {1 2 3} 1] == 2} }
test "catch" {
    set err [catch {exit 0} ret]
    assert "$err == 5" ;# have to use double quotes to interpolate at this level
    assert "$ret == 0" ;# due to assert not upvar'ing
}
test "catch2" {
    set err [catch {exit 1} ret]
    assert "$err == 5"
    assert "$ret == 1"
}

set upvar_test_var 100
test "upvar" {
    upvar upvar_test_var v
    assert "$v == 100"

    set i 100
    proc my_set {name value} {
        upvar $name v
        set v $value
    }
    my_set i 200
    assert "$i == 200"
}

test "label_goto" {
    set count 3

    label l {
        set count [incr $count -1]
        puts "looping $count"
        if {$count > 0} {
            goto l
        }
    }
    assert "$count == 0"
}

test "label_leave" {
    set fail 0
    label l {
        leave l
        set fail 1
    }
    assert "$fail == 0"
}

test "label_nested" {
    set done 0
    label l1 {
        if {$done > 0} {
            leave l1
        }
        label l2 {
            set done 1
            goto l1
        }
    }
    assert "$done == 1"
}

test "label_scope" {
    label l1 { noop }
    assert_error 4 { goto l1 }
}
