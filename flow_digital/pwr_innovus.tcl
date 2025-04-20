# ********************************************************************************
# File             : pwr_innovus.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Setups environment and organizes process;
#                    Entry point power consumption of implemented design
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Include
# Verify the sourcing is not affected by the execution path (creating custom source: csource)
if {0} {
    # Note: for copy-paste purposes in interactive mode
    source "../flow_setup.init.tcl"
} else {
    source [file join [file dirname [info script]] "flow_setup.init.tcl"]
}
linit [file root "[get_db log_file]"]


try {
    # -- Debugging
    set debug.object [lobj "Main" "Check if all sources files are included."]

    ldebug [lobj "Start of Process"]

    # -- Setup
    ldebug [lobj "Flow Setup"]
    csource flow_setup.load.tcl

    # -- Execution
    ldebug [lobj "Initialization"]
    csource flow_innovus.host.tcl
    csource pwr_innovus.directories.tcl
    csource pwr_innovus.initialize.tcl
    ldebug [lobj "Analysis"]
    csource pwr_innovus.activity.tcl
    ldebug [lobj "Reports"]
    csource pwr_innovus.reports.tcl

    ldebug [lobj "End of Process"]

    set code 0
} on error debug.message {
    dict set debug.object message "${debug.message}"
    lerror ${debug.object}

    set code 1
}

# -- Terminate
ldebug [lobj "Return code '$code'"]
lterm $code
exit $code
