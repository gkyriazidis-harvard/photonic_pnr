# ********************************************************************************
# File             : pwr_genus.reports.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Extracts reports of synthesized design
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Reports Extraction"]


set cmd ${instance.command}

report power -header           > [FILE_join "${fd_report}" "report.power.[string tolower $cmd].FINAL.txt"]
report power -by_libcell       > [FILE_join "${fd_report}" "report.power_by_libcell.[string tolower $cmd].FINAL.txt"]
report power -by_leaf_instance > [FILE_join "${fd_report}" "report.power_by_leaf_instance.[string tolower $cmd].FINAL.txt"]


# -- Debugging {unset}
dpop
