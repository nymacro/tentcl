test "if_true"  { if {1 == 1} {} fail }
test "if_false" { if {1 == 2} fail }
test "if_else"  { if {1 == 2} fail else {} }
test "if_else2" { if {1 == 2} fail {} }

test "if_str"   { if {"abc" == "abc"} {} {fail} }
test "if_str!"  { if {"abc" == "cba"} {fail} {} }
