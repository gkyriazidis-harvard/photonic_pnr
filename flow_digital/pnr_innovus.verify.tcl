# ********************************************************************************
# File             : pnr_innovus.verify.tcl
# Version          : 1.0.1, Fri Apr 19 2024
# Description      : Checks for violations of design rules
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Removed 'verifyGeometry' as obselete
# ********************************************************************************


checkPlace
verify_drc -limit 1000000000
#verifyGeometry
verifyConnectivity
if { ${instance.physical.floorplan.routing.antenna} } {
    verifyProcessAntenna
}
