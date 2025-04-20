# ********************************************************************************
# File             : syn_genus.directories.tcl
# Version          : 1.0.1, Fri Mar 21 2025
# Description      : Sets output directories
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Increase path safety
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Directories Setup" "Check 'digital/paths' in 'Project Configuration'."]


# sets output directory (already created in runme_genus.tcl)
set fd_output [FILE_join "${project.digital.paths.synthesis}"]

# creates export directory (transferring information to next steps)
set fd_export [FILE_join "${fd_output}" "export"]
mkdir -p "${fd_export}"

# creates report directory (for analysis)
set fd_report [FILE_join "${fd_output}" "report"]
mkdir -p "${fd_report}"


# -- Debugging {unset}
dpop
