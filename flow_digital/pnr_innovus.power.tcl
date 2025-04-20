# ********************************************************************************
# File             : pnr_innovus.power.tcl
# Version          : 1.0.1, Fri Apr 19 2024
# Description      : Routes power
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added toggle for power routing
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Power Routing" "Check 'physical/floorplan/supplies' in 'Mapping Configuration'."]


foreach supply_tmp ${mapping.physical.floorplan.supplies} {
    set name   [dict get $supply_tmp name]
    set layers [dict get $supply_tmp layers]
    set width  [dict get $supply_tmp width]
    addNet -physical $name
    setNet -net $name -type special
    #setPGPinUseSignalRoute *:$name
    #if {[STRING_empty $layers] || [STRING_empty $width]} {
    #    routePGPinUseSignalRoute -nets $name
    #} else {
    #    add_ndr -name ${name}_ndr -width "$layers $width" -generate_via
    #    routePGPinUseSignalRoute -nets $name -nonDefaultRule ${name}_ndr
    #}
}
if { ${mapping.physical.floorplan.routing.power} } {
    #sroute -connect { blockPin padPin padRing corePin floatingStripe secondaryPowerPin } -allowLayerChange 1 -allowJogging 1
    sroute -connect { blockPin padPin padRing corePin } -allowLayerChange 1 -allowJogging 1
    #editTrim -all
}


# -- Debugging {unset}
dpop
