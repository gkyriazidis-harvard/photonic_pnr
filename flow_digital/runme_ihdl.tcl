#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set design_configuration     [lindex $argv 1]
set mapping_configuration    [lindex $argv 2]
set project_configuration    [lindex $argv 3]

set pins_power [XML_readEntry "/root/pdk/details/powerPins" $technology_configuration]
set tech_libraries [XML_readEntry "/root/pdk/libraries/names/name" $technology_configuration]
set map_libraries [XML_readEntry "/root/mapping/libraries/names/name" $mapping_configuration]
set view_schematics [XML_readEntry "/root/pdk/libraries/views/view\[@type='schematic'\]" $technology_configuration]
set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
set library_name [XML_readEntry "/root/project/digital/library/name" $project_configuration]
set fd_library "libs"
set fd_physical [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='physical'\]/path" \
$project_configuration]
set fd_virtuoso [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]
#set fd_output [XML_readExternal "/root/project/variables" \
#    "/root/project/digital/outputs/output\[@type='library'\]/path" \
#$project_configuration]
set sigs_power [XML_readExternal "/root/project/variables" \
    "/root/pdk/physical/floorplan/rails/rail\[@type='power'\]/signal" \
$technology_configuration]
set sigs_ground [XML_readExternal "/root/project/variables" \
    "/root/pdk/physical/floorplan/rails/rail\[@type='ground'\]/signal" \
$technology_configuration]
if {[STRING_empty $sigs_power]} {
    set sigs_power "dummy"
}
if {[STRING_empty $sigs_ground]} {
    set sigs_ground "dummy"
}

set refLibs [join [concat $tech_libraries $map_libraries] ", "]
set refSchs [join [concat "schematic" $view_schematics] ", "]

exec sed -e "s/\${lib}/${library_name}/g" \
         -e "s/\${refLibs}/${refLibs}/g" \
         -e "s/\${refSchs}/${refSchs}/g" \
         -e "s/\${sigPower}/${sigs_power}/g" \
         -e "s/\${sigGround}/${sigs_ground}/g" \
         "ihdl_parameter.template" \
> "${fd_virtuoso}/ihdl_parameter"

if {$pins_power} {
    set input "${fd_physical}/export/${top}.phys.with_power_pins.v"
} else {
    set input "${fd_physical}/export/${top}.phys.no_power_pins.v"
}

set input [exec realpath "${input}"]
cd "${fd_virtuoso}"
exec ihdl \
    -param "ihdl_parameter" \
    -cdslib "cds.lib" \
    +dumb_sch \
    $input \
>&@stdout
