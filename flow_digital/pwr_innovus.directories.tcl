# ********************************************************************************
# File             : pwr_genus.directories.tcl
# Version          : 1.0.1, Fri Mar 21 2025
# Description      : Sets output directories
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Increase path safety
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Directories Setup" "Check 'digital/paths' in 'Project Configuration'."]


# sets input directory
set fd_synthesis  [FILE_join "${project.digital.paths.synthesis}"]
set fd_physical   [FILE_join "${project.digital.paths.physical}"]
set fd_simulation [FILE_join "${project.digital.paths.simulation}"]

# sets output directory (already created in runme_genus.tcl)
set fd_output [FILE_join "${project.digital.paths.power}"]

# creates report directory (for analysis)
set fd_report [FILE_join "${fd_output}" "report"]
mkdir -p "${fd_report}"


# -- Debugging {unset}
dpop
