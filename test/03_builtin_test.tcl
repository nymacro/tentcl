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
