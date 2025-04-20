# ********************************************************************************
# File             : pnr_innovus.macros.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Places manually-defined macros
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Macros Placement" "Check 'physical/floorplan/cells' in 'Mapping Configuration'."]


if ${mapping.physical.floorplan.cells.enable} {
    set macros ${mapping.physical.floorplan.cells.items}
    
    set dims_width  ${instance.physical.dimensions.width}
    set dims_height ${instance.physical.dimensions.height}
    set margins     ${instance.physical.dimensions.margins}
    set margin_horizontal [dict get $margins horizontal]
    set margin_vertical   [dict get $margins vertical]
    set margin_top    $margin_vertical
    set margin_bottom $margin_vertical
    set margin_left   $margin_horizontal
    set margin_right  $margin_horizontal
    
    set x_low    0
    set x_start  $margin_left
    set x_end    [expr "$margin_left+$dims_width"]
    set x_high   [expr "$margin_left+$dims_width+$margin_right"]
    set x_center [expr "($x_start+$x_end)/2"]
    set y_low    0
    set y_start  $margin_bottom
    set y_end    [expr "$margin_bottom+$dims_height"]
    set y_high   [expr "$margin_bottom+$dims_height+$margin_top"]
    set y_center [expr "($y_start+$y_end)/2"]

    set dims []
    dict set dims X_S $x_start
    dict set dims X_C $x_center
    dict set dims X_E $x_end
    dict set dims Y_S $y_start
    dict set dims Y_C $y_center
    dict set dims Y_E $y_end

    dict set dims x_high $x_high
    dict set dims x_low  $x_low
    dict set dims y_high $y_high
    dict set dims y_low  $y_low

    foreach macro $macros {
        set macro_instance    [dict get $macro instance]
        set macro_x           [dict get $macro x]
        set macro_y           [dict get $macro y]
        set macro_orientation [dict get $macro orientation]
        
        set macro_x [expr "[VARS_apply $macro_x $dims]"]
        set macro_y [expr "[VARS_apply $macro_y $dims]"]

        # Note: apply scale
        if { ${technology.physical.floorplan.scale} == "1.0" } {
            set macro_x_tmp $macro_x
            set macro_y_tmp $macro_y
        } else {
            set macro_x_tmp [expr "$macro_x / ${technology.physical.floorplan.scale}"]
            set macro_y_tmp [expr "$macro_y / ${technology.physical.floorplan.scale}"]
        }
        
        #Example: placeInstance padring_inst/u_pad_corner_bottom_left 138.5 138.5 R180 -placed
        #placeInstance $macro_instance $macro_x $macro_y $macro_orientation -placed
        placeInstance $macro_instance $macro_x_tmp $macro_y_tmp $macro_orientation -fixed

        linfo [lobj "'$macro' at (x,y) = ($macro_x,$macro_y) as '$macro_orientation'"]
    }
}


# -- Debugging {unset}
dpop
