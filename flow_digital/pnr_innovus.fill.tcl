# ********************************************************************************
# File             : pnr_innovus.fill.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Fills metal layers with random shapes;
#                    Useful for density design rules
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Metal Fill" "Check 'floorplan/rails' in 'Technology Configuration'."]


deleteRouteBlk -all

csource pnr_innovus.verify.tcl

if { ${mapping.physical.floorplan.routing.fill} } {
    set rails_signal [list ]
    foreach {rail} ${technology.physical.floorplan.rails} {
        set signal [dict get $rail signal]
        
        set rails_signal [linsert $rails_signal 1 "$signal"]
    }

    addMetalFill -nets $rails_signal

    csource pnr_innovus.verify.tcl
}


# -- Debugging {unset}
dpop
