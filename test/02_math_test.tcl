test ">" {
    assert {[expr 1 > 2] == 0}
    assert {[expr 2 > 1] == 1}
}
test "<" {
    assert {[expr 1 < 2] == 1}
    assert {[expr 2 < 1] == 0}
}
test "==" {
    assert {[expr 1 == 2] == 0}
    assert {[expr 2 == 1] == 0}
    assert {[expr 1 == 1] == 1}
}

test "plus" { assert {[expr 1 + 1] == 2} }
test "minus" { assert {[expr 1 - 1] == 0} }
test "mult" { assert {[expr 1 * 2] == 2} }
test "div" { assert {[expr 2 / 1] == 2} }
test "negative" { assert {[eql [expr 8 * -1] -8]} }
test "negative2" { assert {[eql [add 8 -16] -8]} }

test "expr" {
    set i 100
    assert {[expr {$i == 100}] == 1}
}
