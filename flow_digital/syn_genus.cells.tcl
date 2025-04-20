# ********************************************************************************
# File             : syn_genus.cells.tcl
# Version          : 1.0.0, Fri Mar 8 2024
# Description      : Sets available cells from technology
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Cell Selection"
set debug.object [lobj ${debug.title}]


set cells [get_lib_cells *]

# -- Include
set debug.object [lobj ${debug.title} "Check 'synthesis/cells/include' in 'Mapping Configuration'."]
set buffer [list ]
if { [dict get ${mapping.synthesis.cells.include} enable ] } {
    set_dont_use $cells true
    
    set items [dict get ${mapping.synthesis.cells.include} items]
    foreach item $items {
        set names [get_property $item "name"]
        set_dont_use $names false

        set buffer [lappend buffer $names]
    }

    if { [llength $buffer] > 0 } {
        linfo [lobj "Including cells: $buffer"]
    }
} else {
    set_dont_use $cells false
}

# -- Exclude
set debug.object [lobj ${debug.title} "Check 'synthesis/cells/exclude' in 'Mapping Configuration'."]
set buffer [list ]
if { [dict get ${mapping.synthesis.cells.exclude} enable] } {
    set items [dict get ${mapping.synthesis.cells.exclude} items]
    foreach item $items {
        set names [get_property $item "name"]
        set_dont_use $names true
        
        set buffer [lappend buffer $names]
    }
    
    if { [llength $buffer] > 0 } {
        linfo [lobj "Excluding cells: $buffer"]
    }
}


# -- Debugging {unset}
dpop
