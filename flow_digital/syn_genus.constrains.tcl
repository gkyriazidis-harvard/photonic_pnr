# ********************************************************************************
# File             : syn_genus.constraints.tcl
# Version          : 1.1.1, Fri Mar 21 2025
# Description      : Sets constraints of design
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added support for combinatorial constraints
#     - v1.1.1: Bug fix
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Constraints Definition"
set debug.object [lobj ${debug.title}]


# -- Create Views
# in mmmc mode, need to set the current constraint mode
# in order to add constraints that aren't loaded in the .sdc
set debug.object [lobj ${debug.title} "Check 'general/views' in 'Mapping Configuration'."]
if {${mapping.general.views.enable}} {
    set views []
    foreach {view} ${mapping.general.views.items} {
        set name [dict get $view name]
        if {[dict get $view primary]} {
            set view_primary $name
        }
        lappend views "cm_$name"
    }

    set_interactive_constraint_modes $views
}

# -- Add constraints
set debug.object [lobj ${debug.title} "Check 'general/constraints' in 'Design Configuration'."]
if { ${design.general.constraints.enable} } {
    if { ![STRING_empty ${design.general.constraints.pre}] } {
        puts "reading sdc"
        read_sdc ${design.general.constraints.pre};
    }

    set clk_name []
    
    set_units -time ps -capacitance fF

    # Clock
    set debug.object [lobj ${debug.title} "Check 'general/constraints/@clock' in 'Design Configuration'."]
    foreach {signal} ${design.general.constraints.clocks} {
        set name    [dict get $signal name]
        set primary [dict get $signal primary]
        set period  [dict get $signal period]
        set setup   [dict get $signal setup]
        set hold    [dict get $signal hold]
        set fanout  [dict get $signal fanout]
        set load    [dict get $signal load]
        if {[STRING_empty $period]} {
            set fanout [dict get ${design.general.constraints.defaults.clock} period]
        }
        if {[STRING_empty $setup]} {
            set setup [dict get ${design.general.constraints.defaults.clock} setup]
        }
        if {[STRING_empty $hold]} {
            set hold [dict get ${design.general.constraints.defaults.clock} hold]
        }
        if {[STRING_empty $fanout]} {
            set fanout [dict get ${design.general.constraints.defaults.clock} fanout]
        }
        if {[STRING_empty $load]} {
            set load [dict get ${design.general.constraints.defaults.clock} load]
        }

        if {$primary} {
            set clk_name $name
        }
        
        create_clock [get_ports $name] -period $period
        if {![STRING_empty $setup]} {
            set_clock_uncertainty -setup $setup [get_clocks $name]
        }
        if {![STRING_empty $hold]} {
            set_clock_uncertainty -hold $hold [get_clocks $name]
        }
        if {![STRING_empty $fanout]} {
            set_max_fanout $fanout [get_ports $name]
        }
        if {![STRING_empty $load]} {
            set_max_capacitance $load [get_ports $name]
        }
        #set_ideal_network [get_ports $name]
    }
    if {[string length $clk_name] == 0} {
        #set clk_name [dict get [lindex ${design.general.constraints.clocks} 0] name]
        set clk_name [lindex [get_object_name [get_clocks -quiet]] 0]
    }
    
    # Resets
    set debug.object [lobj ${debug.title} "Check 'general/constraints/@reset' in 'Design Configuration'."]
    foreach {signal} ${design.general.constraints.resets} {
        set name   [dict get $signal name]
        set fanout [dict get $signal fanout]
        set load   [dict get $signal load]
        if {[STRING_empty $fanout]} {
            set fanout [dict get ${design.general.constraints.defaults.reset} fanout]
        }
        if {[STRING_empty $load]} {
            set load [dict get ${design.general.constraints.defaults.reset} load]
        }

        set_false_path -from [get_ports $name]
        if {![STRING_empty $fanout]} {
            set_max_fanout $fanout [get_ports $name]
        }
        if {![STRING_empty $load]} {
            set_max_capacitance $load [get_ports $name]
        }
    }
    
    # Inputs
    set debug.object [lobj ${debug.title} "Check 'general/constraints/@input' in 'Design Configuration'."]
    set default_reference   [dict get ${design.general.constraints.defaults.input} reference]
    set default_destination [dict get ${design.general.constraints.defaults.input} destination]
    if {${design.general.constraints.defaults.automatic}} {
        set reference $default_reference
        set destination $default_destination

        set delay [dict get ${design.general.constraints.defaults.input} delay]
        set max_delay [dict get ${design.general.constraints.defaults.input} max_delay]
        set min_delay [dict get ${design.general.constraints.defaults.input} min_delay]
        if {[STRING_empty $max_delay]} {
            set max_delay $delay
        }

        if {![STRING_empty $max_delay]} {
            if {![STRING_empty $reference]} {
                if {![STRING_empty $min_delay]} {
                    set_input_delay -clock $reference -max $max_delay [all_inputs -no_clocks]
                    set_input_delay -clock $reference -min $min_delay [all_inputs -no_clocks]
                } else {
                    set_input_delay $max_delay -clock $reference [all_inputs -no_clocks]
                }
            } elseif {![STRING_empty $destination]} {
                set_max_delay $max_delay -from [all_inputs -no_clocks] -to $destination
                if {![STRING_empty $min_delay]} {
                    set_min_delay $min_delay -from [all_inputs -no_clocks] -to $destination
                }
            } else {
                if {![STRING_empty $min_delay]} {
                    set_input_delay -max $max_delay [all_inputs -no_clocks]
                    set_input_delay -min $min_delay [all_inputs -no_clocks]
                } else {
                    set_input_delay $max_delay -clock $reference [all_inputs -no_clocks]
                }
            }
        }
        set transition [dict get ${design.general.constraints.defaults.input} transition]
        if {![STRING_empty $transition]} {
            set_input_transition $transition [all_inputs -no_clocks]
        }
        set fanout [dict get ${design.general.constraints.defaults.input} fanout]
        if {![STRING_empty $fanout]} {
            set_max_fanout $fanout [all_inputs -no_clocks]
        }
        set load [dict get ${design.general.constraints.defaults.input} load]
        if {![STRING_empty $load]} {
            set_max_capacitance $load [all_inputs -no_clocks]
        }
    }
    foreach {signal} ${design.general.constraints.inputs} {
        set name        [dict get $signal name]
        set reference   [dict get $signal reference]
        set destination [dict get $signal destination]
        set delay       [dict get $signal delay]
        set max_delay   [dict get $signal max_delay]
        set min_delay   [dict get $signal min_delay]
        set transition  [dict get $signal transition]
        set fanout      [dict get $signal fanout]
        set load        [dict get $signal load]

        if {[STRING_empty $reference]} {
            set reference $default_reference
        }
        if {[STRING_empty $destination]} {
            set destination $default_destination
        }

        if {[STRING_empty $max_delay]} {
            set max_delay $delay
        }

        if {![STRING_empty $max_delay]} {
            if {![STRING_empty $reference]} {
                if {![STRING_empty $min_delay]} {
                    set_input_delay -clock $reference -max $max_delay [get_ports $name]
                    set_input_delay -clock $reference -min $min_delay [get_ports $name]
                } else {
                    set_input_delay $max_delay -clock $reference [get_ports $name]
                }
            } elseif {![STRING_empty $destination]} {
                set_max_delay $max_delay -from [get_ports $name] -to $destination
                if {![STRING_empty $min_delay]} {
                    set_min_delay $min_delay -from [get_ports $name] -to $destination
                }
            } else {
                if {![STRING_empty $min_delay]} {
                    set_input_delay -max $max_delay [get_ports $name]
                    set_input_delay -min $min_delay [get_ports $name]
                } else {
                    set_input_delay $max_delay -clock $reference [get_ports $name]
                }
            }
        }
        if {![STRING_empty $transition]} {
            set_input_transition $transition [get_ports $name]
        }
        if {![STRING_empty $fanout]} {
            set_max_fanout $fanout [get_ports $name]
        }
        if {![STRING_empty $load]} {
            set_max_capacitance $load [get_ports $name]
        }
    }
    if {0} {
        foreach_in_collection port [all_inputs -no_clocks] {
            echo [get_property $port "name"]
            echo [get_property $port "fixed_slew"]
            echo [get_property $port "external_driver"]

            echo [get_property $port "lp_default_toggle_rate"]
            echo [get_property $port "lp_default_probability"]

            echo [get_property $port "lp_asserted_toggle_rate"]
            echo [get_property $port "lp_asserted_probability"]

            echo [get_property $port "lp_computed_toggle_rate"]
            echo [get_property $port "lp_computed_probability"]

            # list_property -type port
            # Property : lp_asserted_probability
            # Property : lp_asserted_toggle_rate
            # Property : system_asserted_probability
            # Property : system_asserted_toggle_rate
            # Property : lp_net_power
            # Property : lp_computed_probability
            # Property : lp_computed_toggle_rate
            # Property : lp_probability_type
            # Property : lp_toggle_rate_type
            # Property : stimulus_activity_id
        }
    }
    
    # Outputs
    set debug.object [lobj ${debug.title} "Check 'general/constraints/@output' in 'Design Configuration'."]
    set default_reference [dict get ${design.general.constraints.defaults.output} reference]
    set default_origin    [dict get ${design.general.constraints.defaults.output} origin]
    if {${design.general.constraints.defaults.automatic}} {
        set reference $default_reference
        set origin $default_origin

        set delay [dict get ${design.general.constraints.defaults.output} delay]
        set max_delay [dict get ${design.general.constraints.defaults.output} max_delay]
        set min_delay [dict get ${design.general.constraints.defaults.output} min_delay]
        if {[STRING_empty $max_delay]} {
            set max_delay $delay
        }

        if {![STRING_empty $max_delay]} {
            if {![STRING_empty $reference]} {
                if {![STRING_empty $min_delay]} {
                    set_output_delay -clock $reference -max $max_delay [all_outputs]
                    set_output_delay -clock $reference -min $min_delay [all_outputs]
                } else {
                    set_output_delay $max_delay -clock $reference [all_outputs]
                }
            } elseif {![STRING_empty $origin]} {
                set_max_delay $max_delay -from $origin -to [all_outputs]
                if {![STRING_empty $min_delay]} {
                    set_min_delay $min_delay -from $origin -to [all_outputs]
                }
            } else {
                if {![STRING_empty $min_delay]} {
                    set_output_delay -max $max_delay [all_outputs]
                    set_output_delay -min $min_delay [all_outputs]
                } else {
                    set_output_delay $max_delay -clock $reference [all_outputs]
                }
            }
        }
        set load [dict get ${design.general.constraints.defaults.output} load]
        if {![STRING_empty $load]} {
            set_load $load [all_outputs]
            #get_property [all_outputs] "external_pin_cap"
        }
    }
    foreach {signal} ${design.general.constraints.outputs} {
        set name      [dict get $signal name]
        set reference [dict get $signal reference]
        set origin    [dict get $signal origin]
        set delay     [dict get $signal delay]
        set max_delay [dict get $signal max_delay]
        set min_delay [dict get $signal min_delay]
        set load      [dict get $signal load]
        
        if {[STRING_empty $reference]} {
            set reference $default_reference
        }
        if {[STRING_empty $origin]} {
            set origin $default_origin
        }

        if {[STRING_empty $max_delay]} {
            set max_delay $delay
        }
        
        if {![STRING_empty $max_delay]} {
            if {![STRING_empty $reference]} {
                if {![STRING_empty $min_delay]} {
                    set_output_delay -clock $reference -max $max_delay [get_ports $name]
                    set_output_delay -clock $reference -min $min_delay [get_ports $name]
                } else {
                    set_output_delay $max_delay -clock $reference [get_ports $name]
                }
            } elseif {![STRING_empty $origin]} {
                set_max_delay $max_delay -from $origin -to [get_ports $name]
                if {![STRING_empty $min_delay]} {
                    set_min_delay $min_delay -from $origin -to [get_ports $name]
                }
            } else {
                if {![STRING_empty $min_delay]} {
                    set_output_delay -max $max_delay [get_ports $name]
                    set_output_delay -min $min_delay [get_ports $name]
                } else {
                    set_output_delay $max_delay -clock $reference [get_ports $name]
                }
            }
        }
        if {![STRING_empty $load]} {
            set_load $load [get_ports $name]
            #get_property [get_ports $name] "external_pin_cap"
        }
    }

    if { ![STRING_empty ${design.general.constraints.post}] } {
        puts "reading sdc"
        read_sdc ${design.general.constraints.post};
    }
}

# Write to .sdc file
# in mmmc mode, need to specify the constraint mode
set debug.object [lobj ${debug.title} "Check 'modules/top' in 'Design Configuration'."]
if {${mapping.general.views.enable}} {
    write_sdc -view "av_$view_primary" > [FILE_join "${fd_export}" "${design.modules.top}.sdc"]
} else {
    write_sdc > [FILE_join "${fd_export}" "${design.modules.top}.sdc"]
}


# -- Debugging {unset}
dpop
