# ********************************************************************************
# File             : pnr_innovus.route.tcl
# Version          : 1.1.3, Wed Apr 23 2025
# Description      : Optimizes routing & placement
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added 1-D strict routing toggle
#     - v1.0.2: Updated top & bottom routing layers commands
#     - v1.1.0: Added optimality toggles
#     - v1.1.1: Added fix toggle
#     - v1.1.2: Added fence option
#     - v1.1.3: Minor tweak
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Place-and-Route Optimization"
set debug.object [lobj ${debug.title}]


set debug.object [lobj ${debug.title} "Check 'physical/floorplan/routing' in 'Mapping Configuration'."]
set layers ${mapping.physical.floorplan.routing.layers}
set first  [dict get $layers first]
set last   [dict get $layers last]
if { ![STRING_empty $first] } {
    setAttribute -net * -bottom_preferred_routing_layer  $first
    setAttribute -net * -preferred_routing_layer_effort hard
    setDesignMode -bottomRoutingLayer $first
    setNanoRouteMode -routeBottomRoutingLayer [lsearch -exact [dbGet head.layers.name] "$first"]
}
if { ![STRING_empty $last] } {
    setAttribute -net * -top_preferred_routing_layer $last
    setAttribute -net * -preferred_routing_layer_effort hard
    setDesignMode -topRoutingLayer $last
    setNanoRouteMode -routeTopRoutingLayer [lsearch -exact [dbGet head.layers.name] "$last"]
}
if { ${mapping.physical.floorplan.routing.strict} } {
    setNanoRouteMode -routeStrictlyHonor1dRouting true
}

setSignoffOptMode -fixGlitch true
if { ${design.general.constraints.enable} } {
    setNanoRouteMode -routeWithTimingDriven true
}
setNanoRouteMode -routeWithSiDriven true

set debug.object [lobj ${debug.title} "Check 'antenna' in 'Technology or Mapping Configuration'."]
if { ${instance.physical.floorplan.routing.antenna} } {
    setNanoRouteMode -drouteFixAntenna true
    setNanoRouteMode -routeInsertAntennaDiode true

    #setNanoRouteMode -routeAntennaCellName ${technology.cells.antenna}
    setNanoRouteMode -quiet -routeAntennaCellName ${technology.cells.antenna}
    set route_antenna_cell_name ${technology.cells.antenna}
    
    setNanoRouteMode -quiet -routeIgnoreAntennaTopCellPin false
    setNanoRouteMode -routeInsertDiodeForClockNets true
}

set debug.object [lobj ${debug.title}]

# -- Detailed Routing
ldebug [lobj "Detailed Routing"]
setNanoRouteMode -envNumberProcessor 8
if {0} {
    # needs special license setup:
    route_opt_design -setup -out_dir [FILE_join "${fd_output}" "route_opt"] -prefix postroute
} else {
    routeDesign -noPlacementCheck
}

if {0} {
    # Note: Power routing was moved to previous step
    csource pnr_innovus.power.tcl
}
csource pnr_innovus.verify.tcl


# -- Post-Route Optimization
if {0} {
    # Note: Not tested
    ldebug [lobj "Post-Route Optimization"]

    setExtractRCMode -effortLevel high
    
    setExtractRCMode \
        -engine        postRoute \
        -total_c_th    0 \
        -relative_c_th 0.03 \
        -coupling_c_th 1
    
    setAnalysisMode \
        -analysisType  onChipVariation \
        -cppr          both
    
    setDelayCalMode \
        -engine        aae \
        -siAware       true

    optDesign -postRoute -outDir [FILE_join "${fd_output}" "postRoute"] -prefix postRoute
}

# -- Post-Route Hold Optimization
if {0} {
    # Note: Not tested
    # TODO: Add corner support
    ldebug [lobj "Post-Route Hold Optimization"]

    setExtractRCMode \
        -effortLevel   high
    
    setExtractRCMode \
        -engine        postRoute \
        -total_c_th    0 \
        -relative_c_th 0.03 \
        -coupling_c_th 1
    
    setOptMode \
        -fixHoldAllowSetupTnsDegrade false \
        -ignorePathGroupsForHold     {default}
    
    optDesign -postRoute -hold -outDir [FILE_join "${fd_output}" "postRoute_hold"] -prefix postRoute_hold
    
    extractRC
    foreach rc_corner [all_rc_corners] {
        rcOut -rc_corner $rc_corner -spef [FILE_join "${fd_output}" "${top_module}.$rc_corner.spef"]
    }
}


# -- Optimization
if { ${design.general.constraints.enable} && ${steps.routing.optimal} } {
    ldebug [lobj "Routing Optimization"]

    setAnalysisMode \
        -analysisType  onChipVariation \
        -cppr          both

    setDelayCalMode \
        -engine        aae \
        -siAware       false

    setOptMode -effort high
    setOptMode -fixDRC true
    setOptMode -honorFence true
    setOptMode -holdTargetSlack 0.0
    setOptMode -setupTargetSlack 0.0

    optDesign -postRoute
    optDesign -postRoute -hold -outDir [FILE_join "${fd_output}" "postRoute_hold"] -prefix postRoute_hold

    csource pnr_innovus.verify.tcl
}


# -- DRC Fix
if { ${steps.routing.fix} } {
    ldebug [lobj "DRC Fix: Post-Route"]

    ecoRoute -target
    refinePlace
    if { ${steps.placement.optimal} } {
        place_opt_design -incremental
    }
    routeDesign -noPlacementCheck

    csource pnr_innovus.verify.tcl
}


# -- Debugging {unset}
dpop
