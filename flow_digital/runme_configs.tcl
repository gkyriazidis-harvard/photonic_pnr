#!/bin/tclsh

# Verify the sourcing is not affected by the execution path
source [file join [file dirname [info script]] "runme_procs.tcl"]

set technology_configuration [lindex $argv 0]
set design_configuration     [lindex $argv 1]
set mapping_configuration    [lindex $argv 2]
set project_configuration    [lindex $argv 3]

try {
    # -- Technology
    if {![file exist $technology_configuration]} {
        throw {""} "Technology Configuration file does not exist."
    }
    if {![XML_checkFormat $technology_configuration]} {
        throw {""} "Technology Configuration file format is incorrect."
    }

    # -- Design
    if {![file exist $design_configuration]} {
        throw {""} "Design Configuration file does not exist."
    }
    if {![XML_checkFormat $design_configuration]} {
        throw {""} "Design Configuration file format is incorrect."
    }

    # -- Mapping
    if {![file exist $mapping_configuration]} {
        throw {""} "Mapping Configuration file does not exist."
    }
    if {![XML_checkFormat $mapping_configuration]} {
        throw {""} "Mapping Configuration file format is incorrect."
    }

    # -- Project
    if {![file exist $project_configuration]} {
        throw {""} "Project Configuration file does not exist."
    }
    if {![XML_checkFormat $project_configuration]} {
        throw {""} "Project Configuration file format is incorrect."
    }

    puts ""
    puts "\[SUCCESS\]"
    puts ""
    
    set code 0
} on error debug.message {
    puts ""
    puts "\[FAILURE\] ${debug.message}"
    puts ""

    set code 1
}

exit $code
