# ********************************************************************************
# File             : pnr_innovus.clock.tcl
# Version          : 1.2.0, Tue Oct 8 2024
# Description      : Synthesizes clock tree;
#                    Reference in:
#                    <innovus-installation>/doc/innovusUG/Clock_Tree_Synthesis.html#ClockTreeSynthesis-QuickStartExample.1
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added non-CTS optimization procedure
#     - v1.2.0: Added enablement toggle
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Clock Tree Synthesis"
set debug.object [lobj ${debug.title}]


set enable ${steps.routing.clock}
if { $enable } {
    if { ${design.general.constraints.enable} } {
        if { [llength [get_clocks]] > 0 } {
            # -- Create clocks
            # Note: No need as the sdc file is being loaded
            if {0} {
                set debug.object [lobj ${debug.title} "Check 'general/constraints/clocks' in 'Design Configuration'."]
                if { ${mapping.general.views.enable} } {
                    set views []
                    foreach {view} ${mapping.general.views.items} {
                        set $name [dict get $view name]
                        if {[dict get $view primary]} {
                            set view_primary $name
                        }
                        lappend views "cm_$name"
                    }
                } else {
                    set views {"cm_tt"}
                    set view_primary "tt"
                }
                set_interactive_constraint_modes $views

                foreach {signal} ${design.general.constraints.clocks} {
                    set name    [dict get $signal name]
                    set primary [dict get $signal primary]
                    set period  [dict get $signal period]
                    set fanout  [dict get $signal fanout]

                    if {$primary} {
                        set clk_name $name
                    }
                    
                    set half_period [expr "0.5 * $period"]
                    
                    create_clock -name $name -period $period -waveform [list 0 $half_period] [get_ports $name]
                }
                if {[string length clk_name] == 0} {
                    set clk_name [dict get [lindex ${design.general.constraints.clocks} 0] name]
                }
            }
            
            set debug.object [lobj ${debug.title}]
            
            # -- Pre-CTS Optimization
            ldebug [lobj "Pre-CTS Optimization"]
            setOptMode -fixDRC
            setOptMode -fixFanoutLoad true
            optDesign -preCTS

            create_ccopt_clock_tree_spec

            # -- Set clock buffer cells
            if { [llength ${technology.cells.clock}] > 0 } {
                set_ccopt_property buffer_cells ${technology.cells.clock}
                set_ccopt_property use_inverters false
                set_ccopt_property inverter_cells [list]
            }

            # -- Execution
            ldebug [lobj "Clock Optimization"]
            ccopt_design -outDir [FILE_join "${fd_output}" "ccopt"] -prefix cts
            
            report_ccopt_clock_trees
            report_ccopt_clock_tree_structure

            # -- Post-CTS Optimization
            ldebug [lobj "Post-CTS Optimization"]
            setOptMode -fixHoldAllowSetupTnsDegrade false -ignorePathGroupsForHold {default}
            setOptMode -fixDRC
            setOptMode -fixFanoutLoad true
            optDesign -postCTS -hold -outDir [FILE_join "${fd_output}" "postCTS"] -prefix postcts_hold

            place_opt_design -incremental
        } else {
            # -- Pre-CTS Optimization
            ldebug [lobj "Pre-CTS Optimization"]
            setOptMode -fixDRC
            optDesign -preCTS
        }
    }
}


# -- Debugging {unset}
dpop
