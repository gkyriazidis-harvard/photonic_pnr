# ********************************************************************************
# File             : flow_setup.design.tcl
# Version          : 1.1.0, Tue Mar 18 2025
# Description      : Loads design configuration file;
#                    Supports XML format
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Fixed bug on empty 'constraints/defaults'
#     - v1.0.2: Fixed 'constraints/defaults' bug
#     - v1.1.0: Added support for combinatorial constraints
# ********************************************************************************


# -- Includes
csource "flow_procs.tcl"


# -- Debugging {set}
dpush
set debug.object [lobj "Design Configuration" "Check XML file."]


# -- Configs
set configuration ${design.configuration}
set verbose 1


# -- Checks
if {![file exist $configuration]} {
    throw {""} "Configuration file does not exist"
}
if {![XML_checkFormat $configuration]} {
    throw {""} "Configuration file format is incorrect."
}


# -- Variables {set}
set design.variables [XML_readVariables "/root/design/variables/variable" \
    "name" \
    "value" \
    $configuration]
if {$verbose} {puts ${design.variables}} {}
set VARS ${design.variables}
puts $VARS

# -- Sources
set design.sources [XML_readConfigs "/root/design/sources/source\[@type='logic'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${design.sources}} {}

# -- Modules
set design.modules.top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $configuration]
if {$verbose} {puts ${design.modules.top}} {}

# -- General
# ---- Supplies
set buffer [XML_readEntries "/root/design/general/supplies/supply" [list \
    "name" \
    "@type" \
    "voltage" \
] " " $configuration]
set design.general.supplies []
foreach {signal} $buffer {
    set temp []
    dict set temp name    [lindex $signal 0]
    dict set temp type    [lindex $signal 1]
    dict set temp voltage [lindex $signal 2]
    lappend design.general.supplies $temp
}
if {$verbose} {puts ${design.general.supplies}} {}
# ---- Constraints
# ------ Enable
set design.general.constraints.enable [BOOL_check [XML_readEntry "/root/design/general/constraints/enable" $configuration]]
if {$verbose} {puts ${design.general.constraints.enable}} {}
# ------ Source
set design.general.constraints.pre [XML_readConfigs "/root/design/general/constraints/pre/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${design.general.constraints.pre}} {}
set design.general.constraints.post [XML_readConfigs "/root/design/general/constraints/post/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${design.general.constraints.post}} {}
# ------ Defaults
# -------- Automatic
set design.general.constraints.defaults.automatic [BOOL_check [XML_readEntry "/root/design/general/constraints/defaults/automatic" $configuration]]
if {$verbose} {puts ${design.general.constraints.defaults.automatic}} {}
# -------- Clock
set buffer [XML_readEntries "/root/design/general/constraints/defaults/signal\[@class='clock'\]/" [list \
    "period" \
    "setup" \
    "hold" \
    "fanout" \
    "load" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp period [lindex $buffer 0]
dict set temp setup  [lindex $buffer 1]
dict set temp hold   [lindex $buffer 2]
dict set temp fanout [lindex $buffer 3]
dict set temp load   [lindex $buffer 4]
set design.general.constraints.defaults.clock $temp
if {$verbose} {puts ${design.general.constraints.defaults.clock}} {}
# -------- Reset
set buffer [XML_readEntries "/root/design/general/constraints/defaults/signal\[@class='reset'\]/" [list \
    "fanout" \
    "load" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp fanout [lindex $buffer 0]
dict set temp load   [lindex $buffer 1]
set design.general.constraints.defaults.reset $temp
if {$verbose} {puts ${design.general.constraints.defaults.reset}} {}
# -------- Input
set buffer [XML_readEntries "/root/design/general/constraints/defaults/signal\[@class='input'\]/" [list \
    "reference" \
    "destination" \
    "delay" \
    "min_delay" \
    "max_delay" \
    "transition" \
    "fanout" \
    "load" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp reference   [lindex $buffer 0]
dict set temp destination [lindex $buffer 1]
dict set temp delay       [lindex $buffer 2]
dict set temp min_delay   [lindex $buffer 3]
dict set temp max_delay   [lindex $buffer 4]
dict set temp transition  [lindex $buffer 5]
dict set temp fanout      [lindex $buffer 6]
dict set temp load        [lindex $buffer 7]
set design.general.constraints.defaults.input $temp
if {$verbose} {puts ${design.general.constraints.defaults.input}} {}
# -------- Output
set buffer [XML_readEntries "/root/design/general/constraints/defaults/signal\[@class='output'\]/" [list \
    "reference" \
    "origin" \
    "delay" \
    "min_delay" \
    "max_delay" \
    "load" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp reference [lindex $buffer 0]
dict set temp origin    [lindex $buffer 1]
dict set temp delay     [lindex $buffer 2]
dict set temp min_delay [lindex $buffer 3]
dict set temp max_delay [lindex $buffer 4]
dict set temp load      [lindex $buffer 5]
set design.general.constraints.defaults.output $temp
if {$verbose} {puts ${design.general.constraints.defaults.output}} {}
# ------ Instances
# -------- Clocks
set buffer [XML_readEntries "/root/design/general/constraints/signal\[@class='clock'\]/" [list \
    "name" \
    "@tag" \
    "period" \
    "setup" \
    "hold" \
    "fanout" \
    "load" \
] " " $configuration]
set design.general.constraints.clocks []
foreach {signal} $buffer {
    set temp []
    dict set temp name    [lindex $signal 0]
    dict set temp primary [expr {[lindex $signal 1] == "primary"}]
    dict set temp period  [lindex $signal 2]
    dict set temp setup   [lindex $signal 3]
    dict set temp hold    [lindex $signal 4]
    dict set temp fanout  [lindex $signal 5]
    dict set temp load    [lindex $signal 6]
    lappend design.general.constraints.clocks $temp
}
if {$verbose} {puts ${design.general.constraints.clocks}} {}
# -------- Resets
set buffer [XML_readEntries "/root/design/general/constraints/signal\[@class='reset'\]/" [list \
    "name" \
    "fanout" \
    "load" \
] " " $configuration]
set design.general.constraints.resets []
foreach {signal} $buffer {
    set temp []
    dict set temp name   [lindex $signal 0]
    dict set temp fanout [lindex $signal 1]
    dict set temp load   [lindex $signal 2]
    lappend design.general.constraints.resets $temp
}
if {$verbose} {puts ${design.general.constraints.resets}} {}
# -------- Inputs
set buffer [XML_readEntries "/root/design/general/constraints/signal\[@class='input'\]/" [list \
    "name" \
    "reference" \
    "destination" \
    "delay" \
    "min_delay" \
    "max_delay" \
    "transition" \
    "fanout" \
    "load" \
] " " $configuration]
set design.general.constraints.inputs []
foreach {signal} $buffer {
    set temp []
    dict set temp name        [lindex $signal 0]
    dict set temp reference   [lindex $signal 1]
    dict set temp destination [lindex $signal 2]
    dict set temp delay       [lindex $signal 3]
    dict set temp min_delay   [lindex $signal 4]
    dict set temp max_delay   [lindex $signal 5]
    dict set temp transition  [lindex $signal 6]
    dict set temp fanout      [lindex $signal 7]
    dict set temp load        [lindex $signal 8]
    lappend design.general.constraints.inputs $temp
}
if {$verbose} {puts ${design.general.constraints.inputs}} {}
# -------- Outputs
set buffer [XML_readEntries "/root/design/general/constraints/signal\[@class='output'\]/" [list \
    "name" \
    "reference" \
    "origin" \
    "delay" \
    "min_delay" \
    "max_delay" \
    "load" \
] " " $configuration]
set design.general.constraints.outputs []
foreach {signal} $buffer {
    set temp []
    dict set temp name        [lindex $signal 0]
    dict set temp reference   [lindex $signal 1]
    dict set temp origin      [lindex $signal 2]
    dict set temp delay       [lindex $signal 3]
    dict set temp min_delay   [lindex $signal 4]
    dict set temp max_delay   [lindex $signal 5]
    dict set temp load        [lindex $signal 6]
    lappend design.general.constraints.outputs $temp
}
if {$verbose} {puts ${design.general.constraints.outputs}} {}

# -- Variables {unset}
unset VARS


# -- Debugging {unset}
dpop
