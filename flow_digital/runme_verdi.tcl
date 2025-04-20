#!/bin/tclsh

source "runme_procs.tcl"

set project_configuration [lindex $argv 0]

set fd_output [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='simulation'\]/path" \
$project_configuration]

cd "${fd_output}"
exec verdi &
