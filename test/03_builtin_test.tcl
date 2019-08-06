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

set loop_count 3
test "label" {
    upvar loop_count loop_count

    label l {
        set loop_count [incr $loop_count -1]
        puts "looping $loop_count"
        if {$loop_count > 0} {
            goto l
        }
    }
}
