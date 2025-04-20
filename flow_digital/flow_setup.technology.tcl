# ********************************************************************************
# File             : flow_setup.technology.tcl
# Version          : 1.1.0, Fri Apr 19 2024
# Description      : Loads technology configuration files;
#                    Supports XML format
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added support for RC table
#     - v1.1.1: Added voltage attribute to 'rails'
# ********************************************************************************


# -- Includes
csource "flow_procs.tcl"


# -- Debugging {set}
dpush
set debug.object [lobj "Technology Configuration" "Check if XML file is proper."]


# -- Configs
set configuration ${technology.configuration}
set verbose 1


# -- Checks
if {![file exist $configuration]} {
    throw {""} "Configuration file does not exist."
}
if {![XML_checkFormat $configuration]} {
    throw {""} "Configuration file format is incorrect."
}


# -- Variables {set}
set technology.variables [XML_readVariables "/root/pdk/variables/variable" \
    "name" \
    "value" \
    $configuration]
if {$verbose} {puts ${technology.variables}} {}
set VARS ${technology.variables}

# -- Details
set technology.details.name [XML_readEntry "/root/pdk/details/name" $configuration]
if {$verbose} {puts ${technology.details.name}} {}
set technology.details.node [XML_readEntry "/root/pdk/details/node" $configuration]
if {$verbose} {puts ${technology.details.node}} {}

# -- Libraries
set technology.libraries.cells [XML_readConfigs "/root/pdk/libraries/library\[@type='cells'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${technology.libraries.cells}} {}
set technology.libraries.profiles [XML_readConfigs "/root/pdk/libraries/library\[@type='profiles'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${technology.libraries.profiles}} {}
set technology.libraries.extraction [XML_readConfigs "/root/pdk/libraries/library\[@type='extraction'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${technology.libraries.extraction}} {}
set technology.libraries.table [XML_readConfigs "/root/pdk/libraries/library\[@type='table'\]/" [list \
   "path" \
   "file" \
] "/" $configuration]
if {$verbose} {puts ${technology.libraries.table}} {}
# ---- Units
set buffer [XML_readEntries "/root/pdk/libraries/units/" [list \
    "time" \
    "resistance" \
    "capacitance" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp time        [lindex $buffer 0]
dict set temp resistance  [lindex $buffer 1]
dict set temp capacitance [lindex $buffer 2]
set technology.libraries.units $temp
if {$verbose} {puts ${technology.libraries.units}} {}

# -- Maps
set technology.maps.layers [XML_readConfigs "/root/pdk/maps/map\[@type='layers-innovus'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${technology.maps.layers}} {}

# -- Cells
set technology.cells.filler [XML_readEntry "/root/pdk/cells/cell\[@type='filler'\]" $configuration]
if {$verbose} {puts ${technology.cells.filler}} {}
set technology.cells.decap [XML_readEntry "/root/pdk/cells/cell\[@type='decap'\]" $configuration]
if {$verbose} {puts ${technology.cells.decap}} {}
set technology.cells.tap [XML_readEntry "/root/pdk/cells/cell\[@type='tap'\]" $configuration]
if {$verbose} {puts ${technology.cells.tap}} {}
set technology.cells.clock [XML_readEntry "/root/pdk/cells/cell\[@type='clock'\]" $configuration]
if {$verbose} {puts ${technology.cells.clock}} {}
set technology.cells.tie.all [XML_readEntry "/root/pdk/cells/cell\[@type='tie'\]" $configuration]
if {$verbose} {puts ${technology.cells.tie.all}} {}
set technology.cells.tie.high [XML_readEntry "/root/pdk/cells/cell\[@type='tie' and @variant='high'\]" $configuration]
if {$verbose} {puts ${technology.cells.tie.high}} {}
set technology.cells.tie.low [XML_readEntry "/root/pdk/cells/cell\[@type='tie' and @variant='low'\]" $configuration]
if {$verbose} {puts ${technology.cells.tie.low}} {}
set technology.cells.antenna [XML_readEntry "/root/pdk/cells/cell\[@type='antenna'\]" $configuration]
if {$verbose} {puts ${technology.cells.antenna}} {}

# -- Physical
# ---- Floorplan
set technology.physical.floorplan.site [XML_readEntry "/root/pdk/physical/floorplan/site" $configuration]
if {$verbose} {puts ${technology.physical.floorplan.site}} {}
set technology.physical.floorplan.track [XML_readEntry "/root/pdk/physical/floorplan/track" $configuration]
if {$verbose} {puts ${technology.physical.floorplan.track}} {}
set technology.physical.floorplan.scale [XML_readEntry "/root/pdk/physical/floorplan/scale" $configuration]
if {[STRING_empty ${technology.physical.floorplan.scale}]} {
    set technology.physical.floorplan.scale 1.0
}
if {$verbose} {puts ${technology.physical.floorplan.scale}} {}
# ------ Rails
set buffer [XML_readEntries "/root/pdk/physical/floorplan/rails/rail/" [list \
    "signal" \
    "@type" \
    "voltage" \
    "width" \
    "layer" \
] " " $configuration]
set technology.physical.floorplan.rails []
foreach {signal} $buffer {
    set temp []
    dict set temp signal  [lindex $signal 0]
    dict set temp type    [lindex $signal 1]
    dict set temp voltage [lindex $signal 2]
    dict set temp width   [lindex $signal 3]
    dict set temp layer   [lindex $signal 4]
    lappend technology.physical.floorplan.rails $temp
}
if {$verbose} {puts ${technology.physical.floorplan.rails}} {}

# -- Variables {unset}
unset VARS


# -- Debugging {unset}
dpop
