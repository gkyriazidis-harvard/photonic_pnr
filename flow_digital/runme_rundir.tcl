#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set mapping_configuration    [lindex $argv 1]
set project_configuration    [lindex $argv 2]

set fd_output [XML_readExternal "/root/project/variables" \
	"/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]
set tech_rundir [XML_readExternal "/root/pdk/variables" \
	"/root/pdk/rundirs/rundir\[@type='virtuoso'\]/path" \
$technology_configuration]
set map_rundir [XML_readExternal "/root/mapping/variables" \
	"/root/mapping/rundirs/rundir\[@type='virtuoso'\]/path" \
$mapping_configuration]

if {[STRING_empty $map_rundir]} {
    set fd_rundir $tech_rundir
} else {
    set fd_rundir $map_rundir
}

exec rm -rf "${fd_output}"

exec cp -r "${fd_rundir}" "${fd_output}"
exec chmod -R 775 "${fd_output}"
