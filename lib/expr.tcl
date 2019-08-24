# redefine expr in tcl
#
# currently doesn't support precedence. Applies right to left.

# convert infix to prefix notation
proc infix {args} {
    proc f {x} {
        when# [eql "" $x] { return {""} }
        return $x
    }
    set len [llength $args]
    # puts "> $len"

    when# {$len < 1} {
        return {""}
    }
    when# {$len == 1} {
        return "const [f $args]"
    }
    when# {$len == 2} {
        return "[f [lindex $args 0]] [f [lindex $args 1]]"
    }
    when# {$len == 3} {
        return "[f [lindex $args 1]] [f [lindex $args 0]] [f [lindex $args 2]]"
    }
    when# {$len > 3} {
        set x "[f [lindex $args 1]] [f [lindex $args 0]]"
        drop 2 args
        return "[f $x] \[[apply infix $args]\]"
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

    # puts [length args]
    set args2 [uplevel { apply expand $args }]
    # puts [length args2]
    # puts $args2
    set e [apply infix $args2]
    # puts $e
    eval $e
}

# puts [expr {2 * 100 + 100}]

# set hi hi
# if {$hi == "hi"} {
#     puts {hi == hi}
# }
# if {"hi" == "hii"} {
#     puts {NOOOO hi != hii}
# }
