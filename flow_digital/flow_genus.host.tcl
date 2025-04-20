# ********************************************************************************
# File             : flow_genus.host.tcl
# Version          : 1.0.2, Sat Jun 8 2024
# Description      : Setups tool for high-performance operation
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added detection for GUI mode
#     - v1.0.2: Fixed unsupported command
# ********************************************************************************


set version_full [get_db program_version]
regexp \[0-9\]+ $version_full version_major

if { $version_major == 19 } {
    set support true
} elseif { $version_major == 20 } {
    set support true
} elseif { $version_major == 21 } {
    set support true
} else {
    set support false
}
if {$support} {
    ldebug [lobj "Running on Cadence Genus v${version_full}: Supported"]
} else {
    lwarn [lobj "Running on Cadence Genus v${version_full}: Untested"]
}

if { $version_major >= 20 } {
    set_multi_cpu_usage -local_cpu 8
}

set batch [expr {![get_db gui_enabled]}]
if { ${batch} } {
    ldebug [lobj "Executing in Batch mode"]
} else {
    ldebug [lobj "Executing in GUI mode"]
}

set application.version.full  $version_full
set application.version.major $version_major
set instance.batch $batch
