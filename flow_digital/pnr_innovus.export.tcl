# ********************************************************************************
# File             : pnr_innovus.export.tcl
# Version          : 1.0.0, Wed Apr 9 2025
# Description      : Export views of placed design
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Export"]


csource pnr_innovus.verify.tcl
if {${steps.export.layout}} {
    csource pnr_innovus.layout.tcl
}
if {${steps.export.netlist}} {
    csource pnr_innovus.netlist.tcl
}
if {${steps.export.model}} {
    csource pnr_innovus.model.tcl
}


# -- Debugging {unset}
dpop
