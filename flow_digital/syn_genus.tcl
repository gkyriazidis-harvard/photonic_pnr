# ********************************************************************************
# File             : syn_genus.tcl
# Version          : 1.1.1, Fri Mar 21 2025
# Description      : Setups environment and organizes process;
#                    Entry point of synthesis
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added batch-mode condition to 'exit' command
#     - v1.1.0: Added report toggles
#     - v1.1.1: Added export toggles
# ********************************************************************************


# -- Include
# Verify the sourcing is not affected by the execution path (creating custom source: csource)
if {0} {
    # Note: for copy-paste purposes in interactive mode
    source "../flow_setup.init.tcl"
} else {
    source [file join [file dirname [info script]] "flow_setup.init.tcl"]
}
linit [file root "[get_db stdout_log]"]

try {
    # -- Debugging
    set debug.object [lobj "Main" "Check if all sources files are included."]

    ldebug [lobj "Start of Process"]

    # -- Setup
    ldebug [lobj "Flow Setup"]
    csource flow_setup.load.tcl

    # -- Steps
    set debug.object [lobj "Steps" "Check 'digital/physical/steps' in 'Project configuration'."]
    set steps.report [dict get ${project.digital.physical.steps.report} enable]
    set steps.export [dict get ${project.digital.physical.steps.export} enable]

    set steps.report.design  [dict get ${project.digital.physical.steps.report} design]
    set steps.report.routing [dict get ${project.digital.physical.steps.report} routing]
    set steps.report.rules   [dict get ${project.digital.physical.steps.report} rules]
    set steps.report.power   [dict get ${project.digital.physical.steps.report} power]
    set steps.report.timing  [dict get ${project.digital.physical.steps.report} timing]
    set steps.export.netlist [dict get ${project.digital.physical.steps.export} netlist]
    set steps.export.model   [dict get ${project.digital.physical.steps.export} model]

    # -- Execution
    ldebug [lobj "Initialization"]
    csource flow_genus.host.tcl
    csource syn_genus.directories.tcl
    csource syn_genus.initialize.tcl
    csource syn_genus.cells.tcl
    ldebug [lobj "Synthesis"]
    csource syn_genus.constrains.tcl
    csource syn_genus.synthesis.tcl
    ldebug [lobj "Results"]
    if {${steps.report}} {
        csource syn_genus.reports.tcl
    }
    if {${steps.export}} {
        if {${steps.export.netlist}} {
            csource syn_genus.netlist.tcl
        }
        if {${steps.export.model}} {
            csource syn_genus.model.tcl
        }
    }

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
if { ${instance.batch} } {
    exit $code
}
