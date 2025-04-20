# ********************************************************************************
# File             : pnr_innovus.tcl
# Version          : 1.1.2, Fri Mar 21 2025
# Description      : Setups environment and organizes process;
#                    Entry point of place-and-route
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added batch-mode condition to 'exit' command
#     - v1.1.0: Added optimality toggles
#     - v1.1.1: Added report toggles
#     - v1.1.2: Added export toggles
# ********************************************************************************


# -- Include
# Verify the sourcing is not affected by the execution path (creating custom source: csource)
if {0} {
    # Note: for copy-paste purposes in interactive mode
    source "../flow_setup.init.tcl"
} else {
    source [file join [file dirname [info script]] "flow_setup.init.tcl"]
}
linit [file root "[get_db log_file]"]

try {
    # -- Debugging
    set debug.object [lobj "Main" "Check if all sources files are included."]

    ldebug [lobj "Start of Process"]

    # -- Setup
    ldebug [lobj "Flow Setup"]
    csource flow_setup.load.tcl

    # -- Steps
    set debug.object [lobj "Steps" "Check 'digital/physical/steps' in 'Project configuration'."]
    set steps.initialize [dict get ${project.digital.physical.steps.initialize} enable]
    set steps.floorplan  [dict get ${project.digital.physical.steps.floorplan}  enable]
    set steps.placement  [dict get ${project.digital.physical.steps.placement}  enable]
    set steps.routing    [dict get ${project.digital.physical.steps.routing}    enable]
    set steps.signoff    [dict get ${project.digital.physical.steps.signoff}    enable]
    set steps.report     [dict get ${project.digital.physical.steps.report}     enable]
    set steps.export     [dict get ${project.digital.physical.steps.export}     enable]

    set steps.placement.optimal [dict get ${project.digital.physical.steps.placement} optimal]
    set steps.routing.clock     [dict get ${project.digital.physical.steps.routing}   clock]
    set steps.routing.optimal   [dict get ${project.digital.physical.steps.routing}   optimal]
    set steps.routing.fix       [dict get ${project.digital.physical.steps.routing}   fix]
    set steps.report.design     [dict get ${project.digital.physical.steps.report}    design]
    set steps.report.routing    [dict get ${project.digital.physical.steps.report}    routing]
    set steps.report.rules      [dict get ${project.digital.physical.steps.report}    rules]
    set steps.report.power      [dict get ${project.digital.physical.steps.report}    power]
    set steps.report.timing     [dict get ${project.digital.physical.steps.report}    timing]
    set steps.export.layout     [dict get ${project.digital.physical.steps.export}    layout]
    set steps.export.netlist    [dict get ${project.digital.physical.steps.export}    netlist]
    set steps.export.model      [dict get ${project.digital.physical.steps.export}    model]

    # -- Execution
    if {${steps.initialize}} {
        ldebug [lobj "Initialization"]
        
        csource flow_innovus.host.tcl

        csource pnr_innovus.directories.tcl
        set instance.initialized false
    }

    if {${steps.floorplan}} {
        ldebug [lobj "Floorplan"]
        
        set pre  [dict get ${project.digital.physical.steps.floorplan} pre]
        set post [dict get ${project.digital.physical.steps.floorplan} post]

        if {![STRING_empty $pre]} {
            csource "$pre"
        }
        csource pnr_innovus.initialize.tcl
        csource pnr_innovus.floorplan.tcl
        csource pnr_innovus.blockages.tcl
        csource pnr_innovus.macros.tcl
        csource pnr_innovus.pins.tcl
        csource pnr_innovus.mesh.tcl
        csource pnr_innovus.power.tcl
        if {![STRING_empty $post]} {
            csource "$post"
        }
        
        saveDesign -no_wait "save.log" [FILE_join "${fd_output}" "${design.modules.top}.floor"]
        
        set instance.initialized true
    } elseif { ${instance.initialized} == false } {
        if {${steps.placement}} {
            restoreDesign [FILE_join "${fd_output}" "${design.modules.top}.floor.dat"] ${design.modules.top}
            set instance.initialized true
        }
    }

    if {${steps.placement}} {
        ldebug [lobj "Placement"]
        
        set pre  [dict get ${project.digital.physical.steps.placement} pre]
        set post [dict get ${project.digital.physical.steps.placement} post]

        if {![STRING_empty $pre]} {
            csource "$pre"
        }
        csource pnr_innovus.placement.tcl
        if {![STRING_empty $post]} {
            csource "$post"
        }
        
        saveDesign -no_wait "save.log" [FILE_join "${fd_output}" "${design.modules.top}.placed"]
        
        set instance.initialized true
    } elseif { ${instance.initialized} == false } {
        if {${steps.routing}} {
            restoreDesign [FILE_join "${fd_output}" "${design.modules.top}.placed.dat"] ${design.modules.top}
            set instance.initialized true
        }
    }

    if {${steps.routing}} {
        ldebug [lobj "Routing"]
        
        set pre  [dict get ${project.digital.physical.steps.routing} pre]
        set post [dict get ${project.digital.physical.steps.routing} post]

        if {![STRING_empty $pre]} {
            csource "$pre"
        }
        csource pnr_innovus.clock.tcl
        csource pnr_innovus.route.tcl
        if {![STRING_empty $post]} {
            csource "$post"
        }
        
        saveDesign -no_wait "save.log" [FILE_join "${fd_output}" "${design.modules.top}.routed"]
        
        set instance.initialized true
    } elseif {! ${instance.initialized}} {
        if {${steps.signoff}} {
            restoreDesign [FILE_join "${fd_output}" "${design.modules.top}.routed.dat"] ${design.modules.top}
            set instance.initialized true
        }
    }

    if {${steps.signoff}} {
        ldebug [lobj "Signoff"]
        
        set pre  [dict get ${project.digital.physical.steps.signoff} pre]
        set post [dict get ${project.digital.physical.steps.signoff} post]

        if {![STRING_empty $pre]} {
            csource "$pre"
        }
        csource pnr_innovus.filler.tcl
        csource pnr_innovus.fill.tcl
        if {![STRING_empty $post]} {
            csource "$post"
        }
        
        saveDesign -no_wait "save.log" [FILE_join "${fd_output}" "${design.modules.top}.signoff"]
        
        set instance.initialized true
    } elseif {! ${instance.initialized}} {
        restoreDesign [FILE_join "${fd_output}" "${design.modules.top}.signoff.dat"] ${design.modules.top}
        set instance.initialized true
    }

    if {${steps.report}} {
        ldebug [lobj "Report"]
        
        csource pnr_innovus.reports.tcl
    }

    if {${steps.export}} {
        ldebug [lobj "Export"]
        
        csource pnr_innovus.verify.tcl
        saveDesign -no_wait "save.log" [FILE_join "${fd_output}" "${design.modules.top}"]
        if {${steps.export.layout}} {
            csource pnr_innovus.layout.tcl
        }
        if {${steps.export.netlist}} {
            csource pnr_innovus.netlist.tcl
        }
        if {${steps.export.model}} {
            csource pnr_innovus.model.tcl
        }
    }

    ldebug [lobj "End of Process"]

    set code 0
} on error debug.message {
    dict set debug.object message "${debug.message}"
    lerror ${debug.object}

    set code 1
}

# -- Terminate
ldebug [lobj "Return code '$code'"]
lterm $code
if { ${instance.batch} } {
    exit $code
}
