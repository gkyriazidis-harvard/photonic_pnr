# ********************************************************************************
# File             : pnr_innovus.placement.tcl
# Version          : 1.2.0, Mon Oct 7 2024
# Description      : Initiates placement;
#                    Supports Fillers (Tap, Decap)
#                    Note: Filler cells skipped till after optimization
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Minor optimization imporvements
#     - v1.1.0: Added layer limitation option
#     - v1.2.0: Added optimality toggles
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Initial Placement"
set debug.object [lobj ${debug.title}]


set debug.object [lobj ${debug.title} "Check 'physical/floorplan/pins' in 'Mapping Configuration'."]
if { ${instance.physical.floorplan.pins.skip.enable} } {
    setPlaceMode -place_global_place_io_pins false
} else {
    setPlaceMode -place_global_place_io_pins true
}

set debug.object [lobj ${debug.title} "Check 'physical/floorplan/placement/utilization' in 'Mapping Configuration'."]
set utilization ${mapping.physical.floorplan.placement.utilization}
if { ![STRING_empty $utilization] } {
    generate_fence -target_util $utilization
}
if { ${mapping.physical.floorplan.placement.uniformity} } {
    setPlaceMode -place_global_uniform_density true
    setPlaceMode -place_global_cong_effort high
}

set debug.object [lobj ${debug.title} "Check 'physical/floorplan/routing' in 'Mapping Configuration'."]
set layers ${mapping.physical.floorplan.routing.layers}
set first  [dict get $layers first]
set last   [dict get $layers last]
if { ![STRING_empty $first] } {
    setDesignMode -bottomRoutingLayer $first
}
if { ![STRING_empty $last] } {
    setDesignMode -topRoutingLayer $last
}

set debug.object [lobj ${debug.title}]
ldebug [lobj "Execution"]
if { ${mapping.general.views.enable} && ${steps.placement.optimal} } {
    set_db opt_fix_fanout_load true
    place_opt_design -out_dir [FILE_join "${fd_output}" "place_opt"] -prefix place
} else {
    place_design
}

# -- Fillers (Tap & Decap)
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/placement/fillers' in 'Mapping Configuration'."]
if { ${mapping.physical.floorplan.placement.fillers.enable} } {
    set tap   [dict get ${mapping.physical.floorplan.placement.fillers.utils} tap]
    set decap [dict get ${mapping.physical.floorplan.placement.fillers.utils} decap]

    setFillerMode -add_fillers_with_drc false
    if { [llength ${technology.cells.tap}] > 0 } {
        if { ![STRING_empty $tap] } {
            addFiller -cell ${technology.cells.tap} -prefix FILLER_TAP_ -util $tap -ecoMode true -check_signal_drc true
        } else {
            addFiller -cell ${technology.cells.tap} -prefix FILLER_TAP_ -ecoMode true -check_signal_drc true
        }
    }

    if { [llength ${technology.cells.decap}] > 0 } {
        if { ![STRING_empty $decap] } {
            addFiller -cell ${technology.cells.decap} -prefix FILLER_DECAP_ -util $decap -ecoMode true -check_signal_drc true
        } else {
            addFiller -cell ${technology.cells.decap} -prefix FILLER_DECAP_ -ecoMode true -check_signal_drc true
        }
        
    }
}

set debug.object [lobj ${debug.title}]

csource pnr_innovus.verify.tcl

if {0} {
    # Note: Doesn't seem to benefit performance or fix DRCs
    place_detail
    setPlaceMode -congEffort high
    place_opt_design -incremental
    congRepair
}


# -- Debugging {unset}
dpop
