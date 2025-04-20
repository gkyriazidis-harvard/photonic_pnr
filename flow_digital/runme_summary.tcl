#!/bin/tclsh

source "runme_procs.tcl"

set project_configuration [lindex $argv 0]

#set input_activity "0.01"
#set input_activity "0.1"
set input_activity "0.5"

set p_period [list 1 10 100 1000]

set fd_innovus [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='physical'\]/path" \
$project_configuration]

set fdne_tmp "${fd_innovus}/report/report.timing.nworst-1.txt"
if { [file exists $fdne_tmp] } {
    set arrival_time [exec grep "^Arrival Time" $fdne_tmp | awk {{print $3 }}]
    puts "    critical path delay (in ns) is: $arrival_time"
}

foreach period_tmp $p_period {
    set fdne_tmp "${fd_innovus}/report/report.power.period-${period_tmp}ns.input_activity-${input_activity}.txt"
    if { [file exists $fdne_tmp] } {
        set power_tmp [exec grep "^Total Power:" $fdne_tmp | awk {{ print $3 }}]
        set leakage_power_tmp [exec grep "^Total Leakage Power:" $fdne_tmp | awk {{ print $4 }}]
        set period_str [format "%+6s" "$period_tmp"]
        puts "    input_activty is ${input_activity}, period is ${period_str} ns --> Total Power (in mW) is: $power_tmp, Leakage Power (in mW) is: ${leakage_power_tmp}"
    }
}
