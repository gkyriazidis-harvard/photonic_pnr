# ********************************************************************************
# File             : flow_setup.project.tcl
# Version          : 1.1.3, Fri Mar 21 2025
# Description      : Loads project configuration file;
#                    Supports XML format
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Fixed boolean check bug in enable toggles
#     - v1.1.0: Added optimality toggles
#     - v1.1.1: Added additional step toggles
#     - v1.1.2: Added reporting toggles
#     - v1.1.3: Added exporting toggles
# ********************************************************************************


# -- Includes
csource "flow_procs.tcl"


# -- Debugging {set}
dpush
set debug.object [lobj "Project Configuration" "Check if XML file is proper."]


# -- Configs
set configuration ${project.configuration}
set verbose 1


# -- Checks
if {![file exist $configuration]} {
    throw {""} "Configuration file does not exist"
}
if {![XML_checkFormat $configuration]} {
    throw {""} "Configuration file format is incorrect."
}


# -- Variables {set}
set project.variables [XML_readVariables "/root/project/variables/variable" \
    "name" \
    "value" \
    $configuration]
if {$verbose} {puts ${project.variables}} {}
set VARS ${project.variables}

# -- Digital
# ---- Steps
# ------ Initialize
set buffer [XML_readEntries "/root/project/digital/physical/steps/initialize" [list \
    "enable" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable [BOOL_check [lindex $buffer 0] 1]
set project.digital.physical.steps.initialize $temp
if {$verbose} {puts ${project.digital.physical.steps.initialize}} {}
# ------ Floorplan
set buffer [XML_readEntries "/root/project/digital/physical/steps/floorplan" [list \
    "enable" \
    "pre" \
    "post" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable  [BOOL_check [lindex $buffer 0] 1]
dict set temp pre     [lindex $buffer 1]
dict set temp post    [lindex $buffer 2]
set project.digital.physical.steps.floorplan $temp
if {$verbose} {puts ${project.digital.physical.steps.floorplan}} {}
# ------ Placement
set buffer [XML_readEntries "/root/project/digital/physical/steps/placement" [list \
    "enable" \
    "optimal" \
    "pre" \
    "post" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable   [BOOL_check [lindex $buffer 0] 1]
dict set temp optimal  [BOOL_check [lindex $buffer 1] 1]
dict set temp pre      [lindex $buffer 2]
dict set temp post     [lindex $buffer 3]
set project.digital.physical.steps.placement $temp
if {$verbose} {puts ${project.digital.physical.steps.placement}} {}
# ------ Routing
set buffer [XML_readEntries "/root/project/digital/physical/steps/routing" [list \
    "enable" \
    "clock" \
    "optimal" \
    "fix" \
    "pre" \
    "post" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable   [BOOL_check [lindex $buffer 0] 1]
dict set temp clock    [BOOL_check [lindex $buffer 1] 1]
dict set temp optimal  [BOOL_check [lindex $buffer 2] 1]
dict set temp fix      [BOOL_check [lindex $buffer 3] 1]
dict set temp pre      [lindex $buffer 4]
dict set temp post     [lindex $buffer 5]
set project.digital.physical.steps.routing $temp
if {$verbose} {puts ${project.digital.physical.steps.routing}} {}
# ------ Signoff
set buffer [XML_readEntries "/root/project/digital/physical/steps/signoff" [list \
    "enable" \
    "pre" \
    "post" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable  [BOOL_check [lindex $buffer 0] 1]
dict set temp pre     [lindex $buffer 1]
dict set temp post    [lindex $buffer 2]
set project.digital.physical.steps.signoff $temp
if {$verbose} {puts ${project.digital.physical.steps.signoff}} {}
# ------ Report
set buffer [XML_readEntries "/root/project/digital/physical/steps/report" [list \
    "enable" \
    "design" \
    "routing" \
    "rules" \
    "power" \
    "timing" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable   [BOOL_check [lindex $buffer 0] 1]
dict set temp design   [BOOL_check [lindex $buffer 1] 1]
dict set temp routing  [BOOL_check [lindex $buffer 2] 1]
dict set temp rules    [BOOL_check [lindex $buffer 3] 1]
dict set temp power    [BOOL_check [lindex $buffer 4] 1]
dict set temp timing   [BOOL_check [lindex $buffer 5] 1]
set project.digital.physical.steps.report $temp
if {$verbose} {puts ${project.digital.physical.steps.report}} {}
# ------ Export
set buffer [XML_readEntries "/root/project/digital/physical/steps/export" [list \
    "enable" \
    "layout" \
    "netlist" \
    "model" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp enable  [BOOL_check [lindex $buffer 0] 1]
dict set temp layout  [BOOL_check [lindex $buffer 1] 1]
dict set temp netlist [BOOL_check [lindex $buffer 2] 1]
dict set temp model   [BOOL_check [lindex $buffer 3] 1]
set project.digital.physical.steps.export $temp
if {$verbose} {puts ${project.digital.physical.steps.export}} {}
# ---- Paths
set project.digital.paths.synthesis [XML_readConfig "/root/project/digital/outputs/output\[@type='synthesis'\]/path" $configuration]
if {$verbose} {puts ${project.digital.paths.synthesis}} {}
set project.digital.paths.physical [XML_readConfig "/root/project/digital/outputs/output\[@type='physical'\]/path" $configuration]
if {$verbose} {puts ${project.digital.paths.physical}} {}
set project.digital.paths.simulation [XML_readConfig "/root/project/digital/outputs/output\[@type='simulation'\]/path" $configuration]
if {$verbose} {puts ${project.digital.paths.simulation}} {}
set project.digital.paths.power [XML_readConfig "/root/project/digital/outputs/output\[@type='power'\]/path" $configuration]
if {$verbose} {puts ${project.digital.paths.power}} {}

# -- Variables {unset}
unset VARS


# -- Debugging {unset}
dpop
