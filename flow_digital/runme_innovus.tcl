#!/bin/tclsh

# Verify the sourcing is not affected by the execution path
source [file join [file dirname [info script]] "runme_procs.tcl"]

set cmd [lindex $argv 0]
set log "innovus_[string tolower $cmd]"

if { $cmd == "batch" } {
    set script [lindex $argv 1]
    set technology_configuration [lindex $argv 2]
    set design_configuration     [lindex $argv 3]
    set mapping_configuration    [lindex $argv 4]
    set project_configuration    [lindex $argv 5]
    
    set fd_output [XML_readExternal "/root/project/variables" \
        "/root/project/digital/outputs/output\[@type='physical'\]/path" \
    $project_configuration]
} else { # $cmd == "interactive"
    set technology_configuration [lindex $argv 1]
    set design_configuration     [lindex $argv 2]
    set mapping_configuration    [lindex $argv 3]
    set project_configuration    [lindex $argv 4]
    
    set fd_output [XML_readExternal "/root/project/variables" \
        "/root/project/digital/outputs/output\[@type='physical'\]/path" \
    $project_configuration]
}
exec mkdir -p "$fd_output"

set technology_configuration [exec realpath "${technology_configuration}"]
set design_configuration     [exec realpath "${design_configuration}"]
set mapping_configuration    [exec realpath "${mapping_configuration}"]
set project_configuration    [exec realpath "${project_configuration}"]
set FLOW_HOME [exec realpath [file dirname [info script]]]
if { $cmd == "batch" } {
    set script [exec realpath "${script}"]
    cd "$fd_output"
    exec innovus -log "${log}" -overwrite \
                 -files "${script}" -batch \
                 -execute "set FLOW_HOME ${FLOW_HOME}; \
                           set technology.configuration ${technology_configuration}; \
                           set design.configuration ${design_configuration}; \
                           set mapping.configuration ${mapping_configuration}; \
                           set project.configuration ${project_configuration};" \
    >&@stdout
} else { # $cmd == "interactive"
    cd "$fd_output"
    exec innovus -log "${log}" -overwrite \
                 -execute "set FLOW_HOME ${FLOW_HOME}; \
                           set technology.configuration ${technology_configuration}; \
                           set design.configuration ${design_configuration}; \
                           set mapping.configuration ${mapping_configuration}; \
                           set project.configuration ${project_configuration}; \
                           win" \
    >&@stdout
}
