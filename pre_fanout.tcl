#!/bin/tclsh

source "runme_procs.tcl"

set design_configuration     [lindex $argv 0]
set project_configuration    [lindex $argv 1]

set fd_base [XML_readExternal "/root/project/variables" \
    "/root/project/digital/paths/path\[@type='base'\]" \
$project_configuration]
set fd_synthesis [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='synthesis'\]/path" \
$project_configuration]

set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]

set source "${fd_base}/${fd_synthesis}/export/${top}.syn.v"
exec cp "$source" "$source.bkp"


set doc.file [open $source r+]
set doc.content [read ${doc.file}]
set doc.lines [split ${doc.content} "\n"]
close ${doc.file}

set doc.indices.title [lsearch -regexp ${doc.lines} "^module $top.*"]
set doc.segments.title [lindex ${doc.lines} ${doc.indices.title}]
#puts ${doc.segments.title}
set doc.signals [lmap element [split [lindex [regexp -inline {.*\((.*)\);} ${doc.segments.title}] 1] ","] {string trim $element}]
#puts ${doc.signals}
set doc.indices.exit [lsearch -regexp ${doc.lines} {endmodule}]
set doc.segments.exit [lindex ${doc.lines} ${doc.indices.exit}]
#puts ${doc.segments.exit}

#set doc.segments.wires [lmap element [lsearch -regexp -all -inline ${doc.lines} {^.*wire .*;}] {string trim $element}]
set doc.indices.wires [lsearch -regexp -all ${doc.lines} {^.*wire .*;}]
#puts ${doc.indices.wires}
set doc.segments.wires [lmap index ${doc.indices.wires} {lindex ${doc.lines} $index}]
#puts ${doc.segments.wires}

set doc.indices.ports [LIST_range [expr "${doc.indices.title}+1"] [expr "[lindex ${doc.indices.wires} 0]-1"]]
set doc.segments.ports [lmap index ${doc.indices.ports} {lindex ${doc.lines} $index}]
#puts ${doc.segments.ports}

set doc.indices.logic [LIST_range [expr "[lindex ${doc.indices.wires} end]+1"] [expr "${doc.indices.exit}-1"]]
set doc.segments.logic [lmap index ${doc.indices.logic} {lindex ${doc.lines} $index}]
#puts ${doc.segments.logic}


set doc.nets [list ]
foreach line ${doc.segments.wires} {
    scan $line {  wire %[^;]} content
    set tmp [lmap element [split $content ","] {string trim $element}]
    set doc.nets [concat ${doc.nets} $tmp]
}
#puts ${doc.nets}

set index_fanout 1
set index_net 1
set doc.segments.extras [list]
foreach net ${doc.nets} {
    #set buffer [regexp -all -inline -line "^.*\\($net\\).*" ${doc.content}]
    set indices [lsearch -regexp -all ${doc.segments.logic} "^(?!.*OUT \\($net\\)).*\\($net\\).*"]
    #puts $indices
    set input $net
    if { [llength $indices] > 1 } {
        #puts $input
        
        set count [expr "[llength $indices]-1"]
        for {set i 0} {$i < $count} {incr i} {
            set n1 "ne${index_net}"
            incr index_net
            set n2 "ne${index_net}"
            incr index_net

            lappend doc.segments.extras.wires   "  wire $n1, $n2;"
            lappend doc.segments.extras.fanouts "  BUF2 fa${index_fanout}(.IN($input), .Y($n1), .Z($n2));"
            incr index_fanout
            
            set index [lindex $indices $i]
            set buffer [lindex ${doc.segments.logic} $index]
            set buffer [string map "($net) ($n1)" $buffer]
            set doc.segments.logic [lreplace ${doc.segments.logic} $index $index $buffer]

            set input $n2
        }
        set index [lindex $indices end]
        set buffer [lindex ${doc.segments.logic} $index]
        set buffer [string map "($net) ($input)" $buffer]
        set doc.segments.logic [lreplace ${doc.segments.logic} $index $index $buffer]
    }
}

set doc.file [open $source w+]
puts ${doc.file} ""
puts ${doc.file} "// Processed by Harvard Nano-Design Group Photonic Pre-Processor"
puts ${doc.file} "// Processed on: [clock format [clock seconds] -format "%b %d %Y %H:%M:%S %Z"]"
puts ${doc.file} ""
puts ${doc.file} ${doc.segments.title}
puts ${doc.file} [join ${doc.segments.ports} "\n"]
puts ${doc.file} [join ${doc.segments.wires} "\n"]
puts ${doc.file} ""
puts ${doc.file} [join ${doc.segments.extras.wires} "\n"]
puts ${doc.file} ""
puts ${doc.file} [join ${doc.segments.logic} "\n"]
puts ${doc.file} ""
puts ${doc.file} [join ${doc.segments.extras.fanouts} "\n"]
puts ${doc.file} ${doc.segments.exit}
close ${doc.file}
