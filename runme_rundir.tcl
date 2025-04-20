#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set mapping_configuration    [lindex $argv 1]
set project_configuration    [lindex $argv 2]

set library_name [XML_readEntry "/root/project/digital/library/name" $project_configuration]
set fd_base [XML_readExternal "/root/project/variables" \
    "/root/project/digital/paths/path\[@type='base'\]" \
$project_configuration]
set fd_digital [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='library'\]/path" \
$project_configuration]
set fd_output [XML_readExternal "/root/project/variables" \
    "/root/project/general/paths/path\[@type='virtuoso'\]" \
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
set fd_library "libs"

if {![file exists "${fd_output}/cds.lib"]} {
    exec cp -r "${fd_rundir}" "${fd_output}"
    exec chmod -R 775 "${fd_output}"
}

exec mkdir -p "${fd_output}/${fd_library}"
exec rm -rf "${fd_output}/${fd_library}/${library_name}"
set fd_path [exec realpath --relative-to=[file normalize "./rundir_virtuoso/libs/"] "flow_digital/${fd_digital}/${library_name}"]
exec ln -s "$fd_path" "${fd_output}/${fd_library}"

#set entry "DEFINE ${library_name} ${fd_base}/${fd_library}/${library_name}"
set entry "DEFINE ${library_name} ${fd_library}/${library_name}"
if { [catch {exec grep -Fxq "${entry}" "${fd_output}/cds.lib"}] } {
    exec echo "${entry}" >> "${fd_output}/cds.lib"
}
