#!/bin/tclsh

source "runme_procs.tcl"

set cmd [lindex $argv 0]

if { $cmd == "batch" } {
    set script [lindex $argv 1]
    set design_configuration     [lindex $argv 2]
    set project_configuration    [lindex $argv 3]
} else { # $cmd == "interactive"
    set design_configuration     [lindex $argv 1]
    set project_configuration    [lindex $argv 2]
}

set fd_base [XML_readExternal "/root/project/variables" \
    "/root/project/digital/paths/path\[@type='base'\]" \
$project_configuration]
set fd_physical [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='physical'\]/path" \
$project_configuration]

set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]

exec cp "${fd_base}/${fd_physical}/export/${top}.gds" "${fd_base}/${fd_physical}/export/${top}.gds.bkp"
if { $cmd == "batch" } {
    set script [exec realpath $script]
    
    cd "${fd_base}/${fd_physical}"
    exec calibredrv ${script} \
    >&@stdout
} else { # $cmd == "interactive"
    set source [exec realpath "${fd_base}/${fd_physical}/${top}.gds"]

    cd "${fd_base}/${fd_physical}"
    exec calibredrv -gui ${source} \
    >&@stdout
}
