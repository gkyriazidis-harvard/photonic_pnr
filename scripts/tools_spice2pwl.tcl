#!/bin/tclsh

# -- CLI Arguments
proc error {message} {
    puts stderr "!ERROR!\n$message"
    exit 1
}
set usage "Usage: tools_spice2pwl <input file> <output file> <signal> \[-verbose\]"

# ---- Parse
if { [lindex $argv 0] == "-help" } {
    puts $usage
    exit 0
}
if { $argc < 3 } {
    error $usage
}

set fdne_input  [lindex $argv 0]
set fdne_output [lindex $argv 1]
set signal      [lindex $argv 2]
set argi_max 2

set argi [lsearch -all $argv "-verbose"]
if { [llength $argi] == 0 } {
    set argi -1
    set verbose 0
} elseif { [llength $argi] == 1 } {
    if { $argi > 2 } {
        set verbose 1
    } else {
        error $usage
    }
} else {
    error $usage
}
set argi_max [expr "max($argi_max,$argi)"]


# -- Global Variables
set lines [list ]
set index -1
set tvector [list ]
set vvector [list ]


# -- Generic Procedures
proc MATH_expr {expression} {
    return [format {%g} [expr "$expression"]]
}

proc REGEX_match {input regex} {
    set groups [regexp -inline $regex $input]
    return [lrange $groups 1 end]
}
proc REGEX_matches {input regex} {
    set groups [regexp -all -inline $regex $input]
    return [lrange $groups 1 end]
}

# -- Local Procedures
proc readFile {} {
    global fdne_input
    variable lines
    
    set filePtr [open "${fdne_input}" r]
    set lines [split [read $filePtr] "\n"]
    close $filePtr
}

proc identifySignal {} {
    global lines signal
    variable index

    for {set i 0} {$i<[llength $lines]} {incr i} {
        set line [lindex $lines $i]
        set start [string range $line 0 [string length $signal]]
        if { "$signal " == $start } {
            set index $i
            break;
        }
    }
}

proc captureData {} {
    global lines index signal
    variable tvector
    variable vvector
    
    set line [lindex $lines $index]
    
    set content [string range $line [string length $signal] end]
    set content [REGEX_match $content " PWL \\( (.+) \\)"]
    set content [lindex $content 0]
    
    set tvector [list ]
    set vvector [list ]
    foreach {t v} $content {
        lappend tvector $t
        lappend vvector $v
    }
}

proc printContent {} {
    global tvector vvector
    
    for {set i 0} {$i<[llength $tvector]} {incr i} {
        puts "@[lindex $tvector $i]=>[lindex $vvector $i]"
    }
}

proc exportFile {} {
    global fdne_output tvector vvector
    
    set filePtr [open "${fdne_output}" w]
    for {set i 0} {$i<[llength $tvector]} {incr i} {
        puts $filePtr "[lindex $tvector $i] [lindex $vvector $i]"
    }
    close $filePtr
}


readFile
#    puts [llength $lines]
identifySignal
#    puts $index
if { $index == -1 } {
    puts "Signal does not exist"
    exit 1
}
#    puts [llength lines]
#    puts [dict get ${_prefix} $timescale]
captureData
if { $verbose } {
    printContent
}
#    puts [llength $tvector]
#    puts [llength $vvector]
exportFile
