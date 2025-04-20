# ********************************************************************************
# File             : pwr_genus.views.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Sets timing views;
#                    Supports multiple views
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


set views []
if {${mapping.general.views.enable}} {  
    # create views
    foreach {view} ${mapping.general.views.items} {
        set name [dict get $view name]
        set opcond [dict get $view opcond]
        
        create_library_set \
            -name "ls_$name" \
            -timing ${instance.libraries.cells}
        
        if {[dict get $view extraction]} {
            create_rc_corner \
                -name "rc_$name" \
                -qrc_tech ${technology.libraries.extraction} \
                -temperature [dict get $view temperature]
        } else {
            create_rc_corner \
                -name "rc_$name" \
                -temperature [dict get $view temperature]
        }
        
        create_timing_condition \
            -name "tc_$name" \
            -library_sets "ls_$name" \
            -opcond [dict get $opcond name] \
            -opcond_library [dict get $opcond library]
        
        create_delay_corner \
            -timing_condition "tc_$name" \
            -name "dc_$name" \
            -rc_corner "rc_$name"
        
        if {${design.general.constraints.enable}} {
            create_constraint_mode \
                -name "cm_$name" \
                -sdc_files [list [FILE_join "${fd_synthesis}" "export" "${design.modules.top}.sdc"] ]
        } else {
            create_constraint_mode \
                -name "cm_$name" \
                -sdc_files [list ]
        }
        
        create_analysis_view \
	        -name "av_$name" \
            -constraint_mode "cm_$name" \
            -delay_corner "dc_$name"
        
        lappend views "av_$name"
    }
} else {
    # default view
    create_library_set \
        -name ls_tt \
        -timing ${instance.libraries.cells}

    create_rc_corner \
        -name rc_tt \
        -T 25.0
    
    create_delay_corner \
        -name dc_tt \
        -library_set ls_tt \
        -rc_corner rc_tt

    create_constraint_mode \
        -name cm_tt \
        -sdc_files [list ]
    
    create_analysis_view \
        -name av_tt \
        -constraint_mode cm_tt \
        -delay_corner dc_tt
    
    lappend views "av_tt"
}

set_analysis_view \
    -setup $views \
    -hold $views
