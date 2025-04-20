# ********************************************************************************
# File             : pnr_innovus.directories.tcl
# Version          : 1.0.1, Fri Mar 21 2025
# Description      : Sets output directories
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Increase path safety
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Directories Setup" "Check 'digital/paths' in 'Project Configuration'."]


set fd_synthesis [FILE_join "${project.digital.paths.synthesis}"]
set fd_output    [FILE_join "${project.digital.paths.physical}"]
#mkdir -p "$fd_output"
set fd_export [FILE_join "${fd_output}" "export"]
mkdir -p $fd_export
set fd_report [FILE_join "${fd_output}" "report"]
mkdir -p $fd_report


# -- Debugging {unset}
dpop
