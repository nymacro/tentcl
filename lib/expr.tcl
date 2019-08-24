# redefine expr in tcl
#
# currently doesn't support precedence. Applies right to left.

# convert infix to prefix notation
proc infix {args} {
    set len [llength $args]
    when# {$len < 1} {
        return ""
    }
    when# {$len == 1} {
        return "const $args"
    }
    when# {$len == 2} {
        return "[lindex $args 0] [lindex $args 1]"
    }
    when# {$len == 3} {
        return "[lindex $args 1] [lindex $args 0] [lindex $args 2]"
    }
    when# {$len > 3} {
        set x "[lindex $args 1] [lindex $args 0]"
        puts $args
        drop 2 args
        # return "$x \[[apply infix $args] $b\]"
        puts $args
        return "$x \[[apply infix $args]\]"
    }

    throw "bad len: $len"
}

proc expr {args} {
    proc const {a} {return $a}
    proc + {a b} {add $a $b}
    proc - {a b} {sub $a $b}
    proc * {a b} {mul $a $b}
    proc / {a b} {div $a $b}
    proc == {a b} {eql $a $b}
    proc != {a b} {not [eql $a $b]}
    proc > {a b} {gt $a $b}
    proc < {a b} {lt $a $b}
    proc >= {a b} {or [gt $a $b] [eql $a $b]}
    proc <= {a b} {or [lt $a $b] [eql $a $b]}

    set args2 [uplevel { apply expand $args }]
    # puts $args2
    set e [apply infix $args2]
    # puts $e
    eval $e
}

puts [expr {2 * 100 + 100}]

set hi hi
if {$hi == "hi"} {
    puts {hi == hi}
}
if {"hi" == "hii"} {
    puts {NOOOO hi != hii}
}
