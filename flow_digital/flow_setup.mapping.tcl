# ********************************************************************************
# File             : flow_setup.mapping.tcl
# Version          : 1.2.2, Tue Mar 18 2025
# Description      : Loads mapping configuration file;
#                    Supports XML format
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Changed structure of 'grid' group
#     - v1.2.0: Added toggles for strict routing, power routing and rails
#     - v1.2.1: Added extra 'signals' option in 'floorplan/grid/stripes'
#     - v1.2.2: Minor tweaks
# ********************************************************************************


# -- Includes
csource "flow_procs.tcl"


# -- Debugging {set}
dpush
set debug.object [lobj "Mapping Configuration" "Check if XML file is proper."]


# -- Configs
set configuration ${mapping.configuration}
set verbose 1


# -- Checks
if {![file exist $configuration]} {
    throw {""} "Configuration file does not exist"
}
if {![XML_checkFormat $configuration]} {
    throw {""} "Configuration file format is incorrect."
}


# -- Variables {set}
set mapping.variables [XML_readVariables "/root/mapping/variables/variable" \
    "name" \
    "value" \
    $configuration]
if {$verbose} {puts ${mapping.variables}} {}
set VARS ${mapping.variables}

# -- General
# ---- Views
set mapping.general.views.enable [BOOL_check [XML_readEntry "/root/mapping/general/views/enable" $configuration] 1]
if {$verbose} {puts ${mapping.general.views.enable}} {}
set buffer [XML_readEntries "/root/mapping/general/views/view/" [list \
    "name" \
    "@tag" \
    "temperature" \
    "opcond/name" \
    "opcond/library" \
    "extraction/enable" \
] " " $configuration]
set mapping.general.views.items []
foreach {signal} $buffer {
    set view []
    dict set view name        [lindex $signal 0]
    dict set view primary     [expr {[lindex $signal 1] == "primary"}]
    dict set view temperature [lindex $signal 2]
    dict set view extraction  [BOOL_check [lindex $signal 5] 0]
    
    set opcond []
    dict set opcond name    [lindex $signal 3]
    dict set opcond library [lindex $signal 4]
    dict set view opcond $opcond

    lappend mapping.general.views.items $view
}
if {$verbose} {puts ${mapping.general.views.items}} {}

# -- Libraries
set mapping.libraries.cells [XML_readConfigs "/root/mapping/libraries/library\[@type='cells'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${mapping.libraries.cells}} {}
set mapping.libraries.profiles [XML_readConfigs "/root/mapping/libraries/library\[@type='profiles'\]/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${mapping.libraries.profiles}} {}

# -- Synthesis
# ---- Cells
set temp []
dict set temp enable [BOOL_check [XML_readEntry "/root/mapping/synthesis/cells\[@type='include'\]/enable" $configuration] 0]
dict set temp items  [XML_readEntry "/root/mapping/synthesis/cells\[@type='include'\]/cell" $configuration]
set mapping.synthesis.cells.include $temp
if {$verbose} {puts ${mapping.synthesis.cells.include}} {}
set temp []
dict set temp enable [BOOL_check [XML_readEntry "/root/mapping/synthesis/cells\[@type='exclude'\]/enable" $configuration] 0]
dict set temp items  [XML_readEntry "/root/mapping/synthesis/cells\[@type='exclude'\]/cell" $configuration]
set mapping.synthesis.cells.exclude $temp
if {$verbose} {puts ${mapping.synthesis.cells.exclude}} {}

# -- Physical
# ---- Dimensions
set mapping.physical.dimensions.width [XML_readEntry "/root/mapping/physical/dimensions/width" $configuration]
if {$verbose} {puts ${mapping.physical.dimensions.width}} {}
set mapping.physical.dimensions.height [XML_readEntry "/root/mapping/physical/dimensions/height" $configuration]
if {$verbose} {puts ${mapping.physical.dimensions.height}} {}
set buffer [XML_readEntries "/root/mapping/physical/dimensions/margins/" [list \
    "horizontal" \
    "vertical" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp horizontal [lindex $buffer 0]
dict set temp vertical   [lindex $buffer 1]
set mapping.physical.dimensions.margins $temp
if {$verbose} {puts ${mapping.physical.dimensions.margins}} {}
# ---- Floorplan
# ------ Initial
set mapping.physical.floorplan.initial [XML_readConfigs "/root/mapping/physical/floorplan/initial/" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.initial}} {}
# ------ Placement
set mapping.physical.floorplan.placement.utilization [XML_readEntry "/root/mapping/physical/floorplan/placement/utilization" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.placement.utilization}} {}
set mapping.physical.floorplan.placement.uniformity [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/placement/uniformity" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.placement.uniformity}} {}
set mapping.physical.floorplan.placement.fillers.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/placement/fillers/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.placement.fillers.enable}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/placement/fillers" [list \
    "tap" \
    "decap" \
    "filler" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp tap    [lindex $buffer 0]
dict set temp decap  [lindex $buffer 1]
dict set temp filler [lindex $buffer 2]
set mapping.physical.floorplan.placement.fillers.utils $temp
if {$verbose} {puts ${mapping.physical.floorplan.placement.fillers.utils}} {}
# ------ Routing
set buffer [XML_readEntries "/root/mapping/physical/floorplan/routing/layers/" [list \
    "first" \
    "last" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp first [lindex $buffer 0]
dict set temp last  [lindex $buffer 1]
set mapping.physical.floorplan.routing.layers $temp
if {$verbose} {puts ${mapping.physical.floorplan.routing.layers}} {}
set mapping.physical.floorplan.routing.strict [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/routing/strict" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.routing.strict}} {}
set mapping.physical.floorplan.routing.fill [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/routing/fill" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.routing.fill}} {}
set mapping.physical.floorplan.routing.power [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/routing/power" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.routing.power}} {}
set mapping.physical.floorplan.routing.antenna [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/routing/antenna" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.routing.antenna}} {}
# ------ Cells
set mapping.physical.floorplan.cells.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/cells/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.cells.enable}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/cells/cell" [list \
    "instance" \
    "location/x" \
    "location/y" \
    "orientation" \
] " " $configuration]
set mapping.physical.floorplan.cells.items []
foreach {signal} $buffer {
    set temp []
    dict set temp instance    [lindex $signal 0]
    dict set temp x           [lindex $signal 1]
    dict set temp y           [lindex $signal 2]
    dict set temp orientation [lindex $signal 3]

    lappend mapping.physical.floorplan.cells.items $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.cells.items}} {}
# ------ Blocks
set mapping.physical.floorplan.blocks.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/blocks/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.blocks.enable}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/blocks/block" [list \
    "@type" \
    "layer" \
    "width" \
    "height" \
    "location/x" \
    "location/y" \
] " " $configuration]
set mapping.physical.floorplan.blocks.items []
foreach {signal} $buffer {
    set temp []
    dict set temp type   [lindex $signal 0]
    dict set temp layer  [lindex $signal 1]
    dict set temp width  [lindex $signal 2]
    dict set temp height [lindex $signal 3]
    dict set temp x      [lindex $signal 4]
    dict set temp y      [lindex $signal 5]

    lappend mapping.physical.floorplan.blocks.items $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.blocks.items}} {}
# ------ Supplies
set buffer [XML_readEntries "/root/mapping/physical/floorplan/supplies/supply" [list \
    "name" \
    "layers" \
    "width" \
] " " $configuration]
set mapping.physical.floorplan.supplies []
foreach {signal} $buffer {
    set temp []
    dict set temp name    [lindex $signal 0]
    dict set temp layers  [lindex $signal 3]
    dict set temp width   [lindex $signal 4]
    lappend mapping.physical.floorplan.supplies $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.supplies}} {}
# ------ Rails
set mapping.physical.floorplan.rails.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/rails/enable" $configuration] 0]
# ------ Rings
set mapping.physical.floorplan.rings.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/rings/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.rings.enable}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/rings/offset" [list \
    "center" \
    "horizontal" \
    "vertical" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp center     [BOOL_check [lindex $buffer 0] 0]
dict set temp horizontal [lindex $buffer 1]
dict set temp vertical   [lindex $buffer 2]
set mapping.physical.floorplan.rings.offset $temp
if {$verbose} {puts ${mapping.physical.floorplan.rings.offset}} {}
set mapping.physical.floorplan.rings.signals [XML_readEntry "/root/mapping/physical/floorplan/rings/signals/signal" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.rings.signals}} {}
set mapping.physical.floorplan.rings.spacing [XML_readEntry "/root/mapping/physical/floorplan/rings/spacing" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.rings.spacing}} {}
set mapping.physical.floorplan.rings.width [XML_readEntry "/root/mapping/physical/floorplan/rings/width" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.rings.width}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/rings/stripes/stripe\[@class='horizontal'\]/" [list \
    "layer" \
    "width" \
    "spacing" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp layer   [lindex $buffer 0]
dict set temp width   [lindex $buffer 1]
dict set temp spacing [lindex $buffer 2]
set mapping.physical.floorplan.rings.stripes.horizontal $temp
if {$verbose} {puts ${mapping.physical.floorplan.rings.stripes.horizontal}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/rings/stripes/stripe\[@class='vertical'\]/" [list \
    "layer" \
    "width" \
    "spacing" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp layer   [lindex $buffer 0]
dict set temp width   [lindex $buffer 1]
dict set temp spacing [lindex $buffer 2]
set mapping.physical.floorplan.rings.stripes.vertical $temp
if {$verbose} {puts ${mapping.physical.floorplan.rings.stripes.vertical}} {}
# ------ Grid
set mapping.physical.floorplan.grid.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/grid/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.grid.enable}} {}
set mapping.physical.floorplan.grid.signals [XML_readEntry "/root/mapping/physical/floorplan/grid/signals/signal" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.grid.signals}} {}
set mapping.physical.floorplan.grid.spacing [XML_readEntry "/root/mapping/physical/floorplan/grid/spacing" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.grid.spacing}} {}
set mapping.physical.floorplan.grid.pitch [XML_readEntry "/root/mapping/physical/floorplan/grid/pitch" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.grid.pitch}} {}
set mapping.physical.floorplan.grid.width [XML_readEntry "/root/mapping/physical/floorplan/grid/width" $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.grid.width}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/grid/stripes/stripe/" [list \
    "signals/signal" \
    "layer" \
    "direction" \
    "width" \
    "spacing" \
    "pitch" \
    "start" \
    "stop" \
    "keepout" \
] " " $configuration]
set mapping.physical.floorplan.grid.stripes []
foreach {stripe} $buffer {
    set temp []
    dict set temp signals   [lindex $stripe 0]
    dict set temp layer     [lindex $stripe 1]
    dict set temp direction [lindex $stripe 2]
    dict set temp width     [lindex $stripe 3]
    dict set temp spacing   [lindex $stripe 4]
    dict set temp pitch     [lindex $stripe 5]
    dict set temp start     [lindex $stripe 6]
    dict set temp stop      [lindex $stripe 7]
    dict set temp keepout   [lindex $stripe 8]
    lappend mapping.physical.floorplan.grid.stripes $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.grid.stripes}} {}
# ------ Pins
set mapping.physical.floorplan.pins.enable [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/pins/enable" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.pins.enable}} {}
set mapping.physical.floorplan.pins.skip [BOOL_check [XML_readEntry "/root/mapping/physical/floorplan/pins/skip" $configuration] 0]
if {$verbose} {puts ${mapping.physical.floorplan.pins.skip}} {}
# ------ Source
set mapping.physical.floorplan.pins.source [XML_readConfigs "/root/mapping/physical/floorplan/pins/source" [list \
    "path" \
    "file" \
] FILE_join $configuration]
if {$verbose} {puts ${mapping.physical.floorplan.pins.source}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/" [list \
    "layer" \
    "width" \
    "height" \
] " " $configuration]
set buffer [lindex $buffer 0]
set temp []
dict set temp layer  [lindex $buffer 0]
dict set temp width  [lindex $buffer 1]
dict set temp height [lindex $buffer 2]
set mapping.physical.floorplan.pins $temp
if {$verbose} {puts ${mapping.physical.floorplan.pins}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin\[side='NONE'\]/" [list \
    "signal" \
    "layer" \
    "width" \
    "height" \
    "location/x" \
    "location/y" \
] " " $configuration]
set mapping.physical.floorplan.pins.fixed []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp width  [lindex $pin 2]
    dict set temp height [lindex $pin 3]
    dict set temp x      [lindex $pin 4]
    dict set temp y      [lindex $pin 5]
    lappend mapping.physical.floorplan.pins.fixed $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.fixed}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin\[side='TOP'\]/" [list \
    "signal" \
    "layer" \
    "width" \
    "height" \
] " " $configuration]
set mapping.physical.floorplan.pins.top []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp width  [lindex $pin 2]
    dict set temp height [lindex $pin 3]
    lappend mapping.physical.floorplan.pins.top $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.top}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin\[side='BOTTOM'\]/" [list \
    "signal" \
    "layer" \
    "width" \
    "height" \
] " " $configuration]
set mapping.physical.floorplan.pins.bottom []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp width  [lindex $pin 2]
    dict set temp height [lindex $pin 3]
    lappend mapping.physical.floorplan.pins.bottom $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.bottom}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin\[side='LEFT'\]/" [list \
    "signal" \
    "layer" \
    "width" \
    "height" \
] " " $configuration]
set mapping.physical.floorplan.pins.left []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp width  [lindex $pin 2]
    dict set temp height [lindex $pin 3]
    lappend mapping.physical.floorplan.pins.left $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.left}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin\[side='RIGHT'\]/" [list \
    "signal" \
    "layer" \
    "width" \
    "height" \
] " " $configuration]
set mapping.physical.floorplan.pins.right []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp width  [lindex $pin 2]
    dict set temp height [lindex $pin 3]
    lappend mapping.physical.floorplan.pins.right $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.right}} {}
set buffer [XML_readEntries "/root/mapping/physical/floorplan/pins/pin/" [list \
    "signal" \
    "layer" \
    "side" \
    "location/x" \
    "location/y" \
] " " $configuration]
set mapping.physical.floorplan.pins.all []
foreach {pin} $buffer {
    set temp []
    dict set temp signal [lindex $pin 0]
    dict set temp layer  [lindex $pin 1]
    dict set temp side   [lindex $pin 2]
    dict set temp x      [lindex $pin 3]
    dict set temp y      [lindex $pin 4]
    lappend mapping.physical.floorplan.pins.all $temp
}
if {$verbose} {puts ${mapping.physical.floorplan.pins.all}} {}

# -- Variables {unset}
unset VARS


# -- Debugging {unset}
dpop
