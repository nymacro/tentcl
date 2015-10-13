pending "list"    { assert {[list 1 2 3] == {1 2 3}} }
test "llength" { assert {[llength {1 2 3}] == 3} }
test "list2"   { assert {[llength [list 1 2 3]] == 3 } }
test "lindex"  { assert {[lindex {1 2 3} 1] == 2} }
test "catch"   { assert {[catch {exit}] == 0} }
