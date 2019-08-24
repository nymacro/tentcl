test "re dot" {
    assert {[eql [regexp {2..} {1234}] 234]}
}
test "re quantifier+" {
    assert {[eql [regexp {1+} {211134}] 111]}
}
test "re quantifier*" {
    assert {[eql [regexp {1*} {11234}] 11]}
}
test "re quantifier* (zero)" {
    assert {[eql [regexp {1*} {234}] ""]}
}

# proper string testing is waiting for string equality...
test "re str" {
    assert {[eql [regexp {...} {abcd}] "abc"]}
}
