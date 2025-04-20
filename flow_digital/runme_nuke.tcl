#!/bin/tclsh

source "runme_procs.tcl"

set project_configuration [lindex $argv 0]

set paths [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output/path" \
$project_configuration]

puts $paths
