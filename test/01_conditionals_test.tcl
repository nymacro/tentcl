test "if_true"  { if {1 == 1} {} fail }
test "if_false" { if {1 == 2} fail }
test "if_else"  { if {1 == 2} fail else {} }
test "if_else2" { if {1 == 2} fail {} }
test "if_elseif" {
    if {1 == 2} {
        fail
    } elseif {1 == 3} {
        fail
    } elseif {1 == 1} {
        # pass
    } else {
        fail
    }
}

pending "if_str"   { if {"abc" == "abc"} {} {fail} }
pending "if_str!"  { if {"abc" == "cba"} {fail} {} }
