# ********************************************************************************
# File             : pwr_innovus.initialize.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Restores implemented design
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Design Initialization" "Check if implemented design exists."]


set instance.command ${command}

# -- Design
if { ${instance.command} == "SYN" } {
    csource "pnr_innovus.initialize.tcl"
} else { # ${instance.command} == "PNR"
    restoreDesign [FILE_join "${fd_physical}" "${design.modules.top}.dat"] ${design.modules.top}
}


# -- Debugging {unset}
dpop
