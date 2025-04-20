# ********************************************************************************
# File             : pnr_innovus.filler.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Places filler cells
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Filler Placement" "Check 'physical/floorplan/placement/fillers' in 'Mapping Configuration'."]


if { ${mapping.physical.floorplan.placement.fillers.enable} } {
    set debug.object [lobj ${debug.title} ]

    set filler [dict get ${mapping.physical.floorplan.placement.fillers.utils} filler]
    
    setFillerMode -add_fillers_with_drc true
    if { [llength ${technology.cells.filler}] > 0 } {
        setFillerMode -core ${technology.cells.filler}
        setFillerMode -corePrefix FILLER_FILL_
        setFillerMode -keepFixed false
        if { ![STRING_empty $filler] } {
            addFiller -util $filler -ecoMode true -check_signal_drc true
        } else {
            addFiller -ecoMode true -check_signal_drc true
        }
    }

    deletePlaceBlockage -all

    #sroute -connect { blockPin padPin padRing corePin } -allowLayerChange 1 -allowJogging 1
    editTrim -all

    #place_opt_design -incremental
    csource pnr_innovus.verify.tcl

    ldebug [lobj "Fix DRC: Post-Filler"]
    ecoRoute -target
    ecoRoute -fix_drc
    editDelete -regular_wire_with_drc
    routeDesign -noPlacementCheck
    if { (${design.general.constraints.enable}) } {
        optDesign -postRoute
        routeDesign -noPlacementCheck
    }

    csource pnr_innovus.verify.tcl
}


# -- Debugging {unset}
dpop
