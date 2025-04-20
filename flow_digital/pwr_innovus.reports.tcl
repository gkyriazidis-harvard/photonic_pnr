# ********************************************************************************
# File             : pwr_innovus.reports.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Extracts reports of implemented design
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Reports Extraction"]


set cmd ${instance.command}

report_power -net > [FILE_join "${fd_report}" "report.power.[string tolower $cmd].FINAL.txt"]
#report_power -rail_analysis_format VS > [FILE_join "${fd_report}" "report.power.[string tolower $cmd].FINAL.txt"]


# -- Debugging {unset}
dpop
