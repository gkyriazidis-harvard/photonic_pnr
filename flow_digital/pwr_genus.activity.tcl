# ********************************************************************************
# File             : pwr_genus.activity.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Reads activity from simulation
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Activity Importing" "Check if simulation results exist."]


set cmd ${instance.command}

read_vcd -static -vcd_scope tb/UUT [FILE_join "${fd_simulation}" "waveforms_[string tolower $cmd].vcd"]


# -- Debugging {unset}
dpop
