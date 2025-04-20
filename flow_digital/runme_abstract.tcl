#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set mapping_configuration    [lindex $argv 1]
set design_configuration     [lindex $argv 2]
set project_configuration    [lindex $argv 3]

set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
set library_name [XML_readEntry "/root/project/digital/library/name" $project_configuration]
set fd_virtuoso [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]

set lis_outputs [XML_readEntries "/root/design/general/constraints/signal\[@class='output'\]/" [list \
    "name"
\] " " $design_configuration]

puts "lis_outputs is '$lis_outputs'"

set output_pattern ""
if {[string length $lis_outputs]} {
    foreach output_tmp $lis_outputs {
	puts "   output: '$output_tmp'"
	# add '*' so that abstract matches any output that starts with the signal name in .xml (e.g., 'out' becomes 'out*')...
	# ... actually the '*' is not needed, abstract can detect buses...
	# append '|' as an "or" in the expression used in abstract options
	#set output_pattern "${output_pattern}${output_tmp}\*|"
	set output_pattern "${output_pattern}${output_tmp}|"
    }
    # remove last character '|'
    set output_pattern [string range $output_pattern 0 [expr [format "%d - 2" [string length $output_pattern]]]]
} else {
    puts "no outputs specified"
}

# debug:
#set output_pattern {accum*}
puts "output_pattern is '$output_pattern'"
#return

set fne_abstract_options "abstract.options"
set fne_output_lef "${top}.abstract.lef"

puts "fd_virtuoso is '${fd_virtuoso}'"

exec sed \
    -e "s/\${library_name}/${library_name}/g" \
    -e "s/\${top}/${top}/g" \
    -e "s/\${fne_abstract_options}/${fne_abstract_options}/g" \
    -e "s/\${fne_output_lef}/${fne_output_lef}/g" \
    -e "s/\${output_pattern}/${output_pattern}/g" \
    "abstract.template.options" \
> "${fd_virtuoso}/${fne_abstract_options}"

exec sed \
    -e "s/\${library_name}/${library_name}/g" \
    -e "s/\${top}/${top}/g" \
    -e "s/\${fne_abstract_options}/${fne_abstract_options}/g" \
    -e "s/\${fne_output_lef}/${fne_output_lef}/g" \
    -e "s/\${output_pattern}/${output_pattern}/g" \
    "abstract.template.replay" \
> "${fd_virtuoso}/abstract.replay"

cd "${fd_virtuoso}"
exec abstract \
    -replay "abstract.replay" \
    -log "abstract.log" \
>&@stdout

