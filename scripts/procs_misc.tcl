# -- LIST
proc LIST_map { buffer expression } {
    set result []
    foreach item $buffer { lappend result [expr [string map {%x $item} $expression]] }
    return $result
}
proc LIST_range {start end {step 1}} {
    if {$step == 0} {
        error "Step cannot be zero."
    }
    set result {}
    if {$step > 0} {
        for {set i $start} {$i <= $end} {incr i $step} {
            lappend result $i
        }
    } else {
        for {set i $start} {$i >= $end} {incr i $step} {
            lappend result $i
        }
    }
    return $result
}


# -- STRING
proc STRING_empty { value } {
    if { [string length $value] == 0 } {
        return 1;
    } else {
        return 0;
    }
}


# -- VARS
proc VARS_apply {expression vars} {
    while {[regexp "\\$\{.*?\}.{0,}" $expression match inner]} {
        set name [string range $match 2 end-1]
        dict set temp $match [dict get $vars $name]
        set expression [string map $temp $expression]
    }
    return $expression
}


# -- STRUCT
proc STRUCT_const { name fields } {
    #foreach field $fields {
    #    uplevel set $name.$field \[ list \]
    #}
    upvar 0 $name srec
    foreach fld $fields {
        set fldn [lindex $fld 0]
        set fldv [lindex $fld 1]
        lappend srec $fldn
        uplevel set $name.$fldn [list $fldv]
    }
    return $name
}

proc STRUCT_read { name field } {
    #return [upvar 1 $name.$field value; puts $value]
    upvar 1 $name.$field value
    return $value
}


# -- FILE
proc FILE_join { args } {
    set base [lindex $args 0]
    set parts [lrange $args 1 end]

    if { [string match "/*" $base] } {
        set result [file join $base {*}$parts]
    } else {
        set result [file normalize [file join [file dirname [info script]] $base {*}$parts]]
    }

    return $result
}


# -- BOOL
proc BOOL_check { value {default "0"} } {
    if { [STRING_empty $value] } {
        return $default
    } else {
        if { $value } {
            return "1"
        } else {
            return "0"
        }
    }
}


# -- PROC
proc PROC_exists { procedure } {
   return uplevel 1 [expr {[llength [info procs $procedure]] > 0}]
}


# -- PINS
proc PINS_expand { signal } {
    set buffer [list]
    set pattern {^(.*)\[([0-9]+):([0-9]+)(?::([0-9]+))?\]$}

    set flag [regexp $pattern $signal full base int1 int2 int3]
    if {$flag} {
        if {$int3 ne ""} {
            set start $int1
            set step  $int2
            set end $int3
        } else {
            set start $int1
            set step  [expr "($int1>$int2 ? -1 : 1)"]
            #set step -1
            set end   $int2
        }

        foreach idx [LIST_range $start $end $step] {
            lappend buffer [format "%s\[%d\]" $base $idx]
        }
    } else {
        set buffer [list $signal]
    }

    return $buffer
}
