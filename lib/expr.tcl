# redefine expr in tcl
#
# currently doesn't support precedence. Applies right to left.

# convert infix to prefix notation
proc infix {args} {
    set len [length args]
    when# {$len < 1} {
        return {""}
    }
    when# {$len == 1} {
        return [list const $args]
    }
    when# {$len == 2} {
        return [list [lindex $args 0] [lindex $args 1]]
    }
    when# {$len == 3} {
        return [list [lindex $args 1] [lindex $args 0] [lindex $args 2]]
    }
    when# {$len > 3} {
        set x [list [lindex $args 1] [lindex $args 0]]
        drop 2 args
        set y [apply infix $args]
        return [list $x \[$y\]]
    }
    throw "error converting expression to infix"
}

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

proc expr {args} {
    set e [uplevel { eval [apply expand [apply infix $args]] }]
    return $e
}
