test "if_true" { if {1 == 1} {puts "pass"} }
test "if_fail" { if {1 == 2} {puts "fail" } }
test "if_else" { if {1 == 2} {puts "fail"} {puts "pass"} }
test "if_else2" { if {1 == 2} {puts "fail"} else {puts "pass"} }
