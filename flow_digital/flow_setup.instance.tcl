# ********************************************************************************
# File             : flow_setup.instance.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Loads instance configuration file;
#                    Supports XML format
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Includes
csource "flow_procs.tcl"


# -- Debugging {set}
dpush
set debug.object [lobj "Instance Configuration" "Check XML file."]


# -- Configs
set verbose 1


# -- Libraries
set instance.libraries.cells [concat ${technology.libraries.cells} ${mapping.libraries.cells}]
if {$verbose} {puts ${instance.libraries.cells}} {}
set instance.libraries.profiles [concat ${technology.libraries.profiles} ${mapping.libraries.profiles}]
if {$verbose} {puts ${instance.libraries.profiles}} {}

# -- Physical
# ---- Floorplan
# ------ Routing
if { [llength ${technology.cells.antenna}] > 0 } {
    set instance.physical.floorplan.routing.antenna ${mapping.physical.floorplan.routing.antenna}
} else {
    set instance.physical.floorplan.routing.antenna 0
}
if {$verbose} {puts ${instance.physical.floorplan.routing.antenna}} {}


# -- Debugging {unset}
dpop
