test "re dot" { assert {[regexp {2..} {1234}] == 234} }
test "re quantifier+" {
    assert {[regexp {1+} {211134}] == 111}
}
test "re quantifier*" {
    assert {[regexp {1*} {11234}] == 11}
}
test "re quantifier* (zero)" {
    assert {[regexp {1*} {234}] == 0}
}

# proper string testing is waiting for string equality...
pending "re str" { regexp {...} {abcd} }
