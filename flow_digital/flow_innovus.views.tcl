# ********************************************************************************
# File             : flow_innovus.views.tcl
# Version          : 1.1.1, Fri Mar 21 2025
# Description      : Setups tool for high-performance operation
#
# Flow Step        : General Flow
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added support for capTbl
#     - v1.1.1: Save views names
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
                -qx_tech_file ${technology.libraries.extraction} \
                -T [dict get $view temperature]
        } else {
            create_rc_corner \
                -name "rc_$name" \
                -cap_table ${technology.libraries.table} \
                -T [dict get $view temperature]
        }

        create_delay_corner \
            -name "dc_$name" \
            -library_set "ls_$name" \
            -opcond [dict get $opcond name] \
            -opcond_library [dict get $opcond library] \
            -rc_corner "rc_$name"

        pwd
        create_constraint_mode \
            -name "cm_$name" \
            -sdc_files [list [FILE_join "${fd_synthesis}" "export" "${design.modules.top}.sdc"] ]
        
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
        -sdc_files [list [FILE_join "${fd_synthesis}" "export" "${design.modules.top}.sdc"] ]
    
    create_analysis_view \
        -name av_tt \
        -constraint_mode cm_tt \
        -delay_corner dc_tt
    
    lappend views "av_tt"
}

# Note: has to be outside the if statement blocks to avoid innovus error {missing close-brace, invalid command '{'}
set_analysis_view \
    -setup $views \
    -hold $views

set instance.views $views
