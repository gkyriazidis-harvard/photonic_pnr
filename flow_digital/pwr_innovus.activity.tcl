# ********************************************************************************
# File             : pwr_innovus.activity.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Reads activity from simulation
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Activity Importing" "Check if simulation results exist."]


set cmd ${instance.command}

set_default_switching_activity -reset
set_default_switching_activity -input_activity 0.5 -period 10.0 -global_activity 0.5

read_activity_file -reset
read_activity_file -format VCD -scope tb/UUT \
                   -start {} -end {} -block {} [FILE_join "${fd_simulation}" "waveforms_[string tolower $cmd].vcd"]

set_power -reset
set_powerup_analysis -reset
set_dynamic_power_simulation -reset


# -- Debugging {unset}
dpop
