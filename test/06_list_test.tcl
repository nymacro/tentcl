test "list_type" {
    set l [list 1 2 3]
    set eq [eql [typeof l] "LIST"]
    assert "$eq == 1"
}

pending "list" { assert {[list 1 2 3] == {1 2 3}} }
pending "listne" { assert {[list 1 2 3] != {1 2 3}} }
test "llength" { assert {[llength {1 2 3}] == 3} }
test "list2"   { assert {[llength [list 1 2 3]] == 3 } }
test "lindex"  { assert {[lindex {1 2 3} 1] == 2} }

test "take" {
    set x [take 1 {1 2 3}]
    assert "$x == 1"
}

test "take2" {
    set x [take 2 {1 2 3}]
    set e [eql $x "1 2"]
    assert "$e == 0"
}
