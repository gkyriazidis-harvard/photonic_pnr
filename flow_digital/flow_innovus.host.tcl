# ********************************************************************************
# File             : flow_innovus.host.tcl
# Version          : 1.0.1, Sun Jun 2 2024
# Description      : Setups tool for high-performance operation
#
# Flow Step        : General Flow
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added detection for GUI mode
# ********************************************************************************


set version_full [get_db program_version]
regexp \[0-9\]+ $version_full version_major

if { $version_major == 20 } {
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
    setDistributeHost -local
    setMultiCpuUsage \
        -localCpu 8 \
        -remoteHost 0 \
        -cpuPerRemoteHost 1
}

set tmp [uiGet main -visible]
if {[STRING_empty $tmp]} {
    set batch 1
} else {
    set batch [expr {!$tmp}]
}
if { ${batch} } {
    ldebug [lobj "Executing in Batch mode"]
} else {
    ldebug [lobj "Executing in GUI mode"]
}

set application.version.full  $version_full
set application.version.major $version_major
set instance.batch $batch
