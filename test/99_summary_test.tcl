puts
puts "Summary:"
if {$tests_pass    > 0} { puts "* Passed:  $tests_pass" }
if {$tests_pending > 0} { puts "* Pending: $tests_pending" }
if {$tests_fail    > 0} { puts "* Failed:  $tests_fail" }

exit [expr $tests_fail != 0]
