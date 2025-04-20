# ********************************************************************************
# File             : pnr_innovus.model.tcl
# Version          : 1.0.0, Fri Mar 21 2025
# Description      : Exports timing/power model of design
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Model Export"
set debug.object [lobj ${debug.title}]


do_extract_model \
    [FILE_join "${fd_output}" "${design.modules.top}.lib"] \
    -view [lindex ${instance.views} 0]
file rename -force "${design.modules.top}.lib" "${fd_export}"


# -- Debugging {unset}
dpop
