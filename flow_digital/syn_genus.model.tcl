# ********************************************************************************
# File             : syn_genus.model.tcl
# Version          : 1.0.0, Fri Mar 21 2025
# Description      : Exports timing/power model of design
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Model Export"
set debug.object [lobj ${debug.title}]


write_lib_lef \
    -lib [FILE_join "${fd_output}" "${design.modules.top}"]
file rename -force "${design.modules.top}.lib" "${fd_export}"


# -- Debugging {unset}
dpop
