#!/bin/tclsh

source "runme_procs.tcl"

set project_configuration [lindex $argv 0]

set fd_rundir [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]

cd "${fd_rundir}"; exec virtuoso & >&@stdout
