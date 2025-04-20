#!/bin/tclsh

set _prefix {
    "y" "e-24"  # yocto
    "z" "e-21"  # zepto
    "a" "e-18"  # atto
    "f" "e-15"  # femto
    "p" "e-12"  # pico
    "n" "e-9"   # nano
    "u" "e-6"   # micro
    "m" "e-3"   # mili
    "c" "e-2"   # centi
    "d" "e-1"   # deci
    "k" "e3"    # kilo
    "M" "e6"    # mega
    "G" "e9"    # giga
    "T" "e12"   # tera
    "P" "e15"   # peta
    "E" "e18"   # exa
    "Z" "e21"   # zetta
    "Y" "e24"   # yotta
}


# -- CLI Arguments
proc error {message} {
    puts stderr "!ERROR!\n$message"
    exit 1
}
set usage "Usage: tools_vcd2spice <input file> <output file> <switch time> (\[-expand\] & \[-scale ?\]) \[-verbose\] \[-hierarchy\] \[{selection}\]"

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
set tswitch     [lindex $argv 2]
set argi_max 2

set argi [lsearch -all $argv "-scale"]
if { [llength $argi] == 0 } {
    set argi -1
    set vscale 1
} elseif { [llength $argi] == 1 } {
    if { $argi > 2 } {
        set vscale [lindex $argv [expr "$argi+1"]]
    } else {
        error $usage
    }
} else {
    error $usage
}
set argi_max [expr "max($argi_max,$argi)"]

set argi [lsearch -all $argv "-expand"]
if { [llength $argi] == 0 } {
    set argi -1
    set expand 0
} elseif { [llength $argi] == 1 } {
    if { $argi > 2 } {
        set expand 1
    } else {
        error $usage
    }
} else {
    error $usage
}
set argi_max [expr "max($argi_max,$argi)"]

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

set argi [lsearch -all $argv "-hierarchy"]
if { [llength $argi] == 0 } {
    set argi -1
    set hierarchy 0
} elseif { [llength $argi] == 1 } {
    if { $argi > 2 } {
        set hierarchy 1
    } else {
        error $usage
    }
} else {
    error $usage
}
set argi_max [expr "max($argi_max,$argi)"]

set selection [lrange $argv [expr "$argi_max+1"] end]

# ---- Check
if { (!$expand) && ($vscale != 1) } {
    error $usage
}


# -- Global Variables
set lines [list ]
set transition 0
set timescale ""
set signals [dict create]
set cvector [list ]
set nvector [list ]
set svector [list ]
set tvectors [list ]
set vvectors [list ]


# -- Generic Procedures
proc MATH_expr {expression} {
    #return [format {%g} [expr "$expression"]]
    return [expr "$expression"]
}

proc REGEX_match {input regex} {
    set groups [regexp -inline $regex $input]
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

proc transitionDetect {} {
    global lines
    variable transition

    for {set i 0} {$i<[llength $lines]} {incr i} {
        if { [lindex $lines $i] == "\$enddefinitions \$end" } {
            set transition [expr $i+1]
            break
        }
    }
}

proc collectSignals {} {
    global lines transition hierarchy
    variable signals
    
    set modules [list ]
    for {set i 0} {$i<$transition} {incr i} {
        set mgroups [REGEX_match [lindex $lines $i] {\$scope module (\w+) \$end}]
        if {[llength $mgroups] != 0} {
            lappend modules [lindex $mgroups 0]
            
            incr i
            while {$i < $transition} {
                set sgroups [REGEX_match [lindex $lines $i] {\$var (?:reg|wire) (\d+)\s(\S+)\s([a-zA-Z]+\S*) (?:\[\S+\] )?\$end}]
                if {[llength $sgroups] != 0} {
                    if { ! [dict exists $signals [lindex $sgroups 1]] } {
                        set temp []
                        if { $hierarchy } {
                            dict set temp name "[join $modules .].[lindex $sgroups 2]"
                        } else {
                            dict set temp name "[lindex $sgroups 2]"
                        }
                        dict set temp size [lindex $sgroups 0]
                        
                        dict set signals [lindex $sgroups 1] $temp
                    }
                    incr i
                } else {
                    break
                }
            }
        }

        if { [lindex $lines $i] == "\$upscope \$end" } {
            set modules [lrange $modules 0 end-1]
        }
    }
}

proc readTimescale {} {
    global lines transition
    variable timescale
    
    for {set i 0} {$i<$transition} {incr i} {
        if { [lindex $lines $i] == "\$timescale" } {
            while { [lindex $lines $i] != "\$end" } {
                set tgroups [REGEX_match [lindex $lines $i] {\d+(\w)s}]
                if {[llength $tgroups] != 0} {
                    set timescale $tgroups
                    return
                }
                incr i
            }
        }
    }
}

proc printContent {} {
    global lines transition timescale signals
    
    for {set i $transition} {$i<[llength $lines]} {incr i} {
        set tgroups [REGEX_match [lindex $lines $i] {\#(\d+)}]
        if {[llength $tgroups] != 0} {
            puts "${tgroups}${timescale}sec"
            continue
        }
        
        set vgroups [REGEX_match [lindex $lines $i] {(^[01bx]{2,}|^[x01])\s?(\S)}]
        if {[llength $vgroups] != 0} {
            set char  [lindex $vgroups 1]
            set value [lindex $vgroups 0]
            
            puts "[dict get [dict get $signals $char] name]=>$value"
            continue
        }
    }
}

proc captureData {} {
    global lines transition timescale _prefix signals tswitch
    variable cvector
    variable nvector
    variable svector
    variable tvectors
    variable vvectors
    
    set cvector [dict keys $signals]
    set nvector [list ]
    set svector [list ]
    foreach char $cvector {
        set temp [dict get $signals $char]
        lappend nvector [dict get $temp name]
        lappend svector [dict get $temp size]
    }
    set count [llength $cvector]
    
    set tmp_tvectors [dict create]
    set tmp_vvectors [dict create]
    set time_prev  [lrepeat $count ""]
    set value_prev [lrepeat $count ""]
    for {set i $transition} {$i<[llength $lines]} {incr i} {
        set tgroups [REGEX_match [lindex $lines $i] {\#(\d+)}]
        if {[llength $tgroups] != 0} {
            set time [format {%g} "${tgroups}[dict get ${_prefix} $timescale]"]
            continue
        }
        
        set vgroups [REGEX_match [lindex $lines $i] {(^[01bx]{2,}|^[x01])\s?(\S)}]
        if {[llength $vgroups] != 0} {
            set char  [lindex $vgroups 1]
            set value [lindex $vgroups 0]
            
            set j [lsearch $cvector "\\$char"]
            if { [string length [lindex $value_prev $j]] == 0 } {
                dict lappend tmp_tvectors $j $time
                dict lappend tmp_vvectors $j $value
            } elseif { [expr "$time<$tswitch"] } {
                dict lappend tmp_tvectors $j $time
                dict lappend tmp_vvectors $j $value
            } else {
                set time1 [MATH_expr "$time-($tswitch)/2"]
                set time2 [MATH_expr "$time+($tswitch)/2"]
                
                dict lappend tmp_tvectors $j $time1
                dict lappend tmp_vvectors $j [lindex $value_prev $j]
                
                dict lappend tmp_tvectors $j $time2
                dict lappend tmp_vvectors $j $value
            }
            
            lset time_prev $j $time
            lset value_prev $j $value
            
            continue
        }
    }
    
    set tvectors [list ]
    set vvectors [list ]
    for {set i 0} {$i<$count} {incr i} {
        lappend tvectors [dict get $tmp_tvectors $i]
        lappend vvectors [dict get $tmp_vvectors $i]
    }
}

proc expandBus {} {
    global svector
    variable nvector
    variable tvectors
    variable vvectors
    
    set indices [list ]
    for {set i 0} {$i<[llength $nvector]} {incr i} {
        if { [lindex $svector $i] > 1 } {
            lappend indices $i
        }
    }
    
    foreach i $indices {
        set size    [lindex $svector $i]
        set vvector [lindex $vvectors $i]        

        set tmp_nvector  [list ]
        set tmp_tvectors [list ]
        set tmp_vvectors [dict create]
        for {set j 0} {$j<$size} {incr j} {
            lappend tmp_nvector  "[lindex $nvector $i]\[$j\]"
            lappend tmp_tvectors [lindex $tvectors $i]
            dict set tmp_vvectors $j [list ]
        }
        
        for {set j 0} {$j<[llength $vvector]} {incr j} {
            set vgroups [REGEX_match [lindex $vvector $j] {^b([x01]+)}]
            if {[llength $vgroups] != 0} {
                set value $vgroups
                
                set value [string reverse $value]
                for {set k 0} {$k<$size} {incr k} {
                    set tmp "x"
                    if { $k < [string length $value] } {
                        set tmp [string index $value $k]
                    }
                    dict lappend tmp_vvectors $k $tmp
                }
            }
        }
        
        for {set j 0} {$j<$size} {incr j} {
            lappend nvector  [lindex $tmp_nvector $j]
            lappend tvectors [lindex $tmp_tvectors $j]
            lappend vvectors [dict get $tmp_vvectors $j]
        }
    }
    
    foreach i [lreverse $indices] {
        set nvector  [lreplace $nvector $i $i]
        set tvectors [lreplace $tvectors $i $i]
        set vvectors [lreplace $vvectors $i $i]
    }
}

proc exportFile {} {
    global fdne_output selection nvector tvectors vvectors vscale
    
    set count [llength $nvector]
    
    set indices [list ]
    if { [llength $selection] == 0 } {
        for {set i 0} {$i<$count} {incr i} {
            lappend indices $i
        }
    } else {
        for {set i 0} {$i<[llength $selection]} {incr i} {
            lappend indices [lsearch $nvector [lindex $selection $i]]
        }
    }
    
    set filePtr [open "${fdne_output}" w]
    foreach i $indices {
        set tvector [lindex $tvectors $i]
        set vvector [lindex $vvectors $i]
        
        set content "[lindex $nvector $i] "
        append content "PWL ( "
        for {set j 0} {$j<[llength $tvector]} {incr j} {
            set time  [lindex $tvector $j]
            if {[catch {MATH_expr "[lindex $vvector $j]*$vscale"} value]} {
                set value [lindex $vvector $j]
            }
            append content "$time $value "
        }
        append content ")"
        
        puts $filePtr $content
    }
    close $filePtr
}


readFile
#    puts [llength $lines]
transitionDetect
#    puts $transition
collectSignals
#    puts $signals
readTimescale
#    puts $timescale
#    puts [dict get ${_prefix} $timescale]
if { $verbose } {
    printContent
}
captureData
#    puts [llength $nvector]
#    puts [llength $tvectors]
#    puts [llength $vvectors]
if { $expand } {
    expandBus
}
#    puts $nvector
#    puts $tvectors
#    puts $vvectors
exportFile
