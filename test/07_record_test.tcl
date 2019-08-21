test "record-basics" {
    set rt [make-record-type "test-record" a b c]
    set r [make-record rt]
    assert {[eql [attributes r] "a b c"] == 1}
}
