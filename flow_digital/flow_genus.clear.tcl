# ********************************************************************************
# File             : flow_genus.clear.tcl
# Version          : 1.0.0, Fri Apr 19 2024
# Description      : Clears initialized designs
#
# Flow Step        : General Flow
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


set designs [get_db designs]
foreach design $designs {
    reset_design $design
    delete_obj $design
}
