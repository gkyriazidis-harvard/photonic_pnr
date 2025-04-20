# ********************************************************************************
# File             : pnr_innovus.blockages.tcl
# Version          : 0.1.1, Mon Apr 14 2025
# Description      : Places manually-defined layout blockages
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v0.1.0: Initial release
#     - v0.1.1: Fixed missing layer bug
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Blockages Placement" "Check 'physical/floorplan/blocks' in 'Mapping Configuration'."]


if ${mapping.physical.floorplan.blocks.enable} {
    set blockages ${mapping.physical.floorplan.blocks.items}

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

    foreach blockage $blockages {
        set blockage_type   [dict get $blockage type]
        set blockage_layer  [dict get $blockage layer]
        set blockage_width  [dict get $blockage width]
        set blockage_height [dict get $blockage height]
        set blockage_x      [dict get $blockage x]
        set blockage_y      [dict get $blockage y]
        
        set blockage_width  [expr "[VARS_apply $blockage_width $dims]"]
        set blockage_height [expr "[VARS_apply $blockage_height $dims]"]
        set blockage_x      [expr "[VARS_apply $blockage_x $dims]"]
        set blockage_y      [expr "[VARS_apply $blockage_y $dims]"]

        # Note: apply scale
        if { ${technology.physical.floorplan.scale} == "1.0" } {
            set blockage_width_tmp  $blockage_width
            set blockage_height_tmp $blockage_height
            set blockage_x_tmp $blockage_x
            set blockage_y_tmp $blockage_y
        } else {
            set blockage_width_tmp  [expr "$blockage_width / ${technology.physical.floorplan.scale}"]
            set blockage_height_tmp [expr "$blockage_height / ${technology.physical.floorplan.scale}"]
            set blockage_x_tmp [expr "$blockage_x / ${technology.physical.floorplan.scale}"]
            set blockage_y_tmp [expr "$blockage_y / ${technology.physical.floorplan.scale}"]
        }

        if { $blockage_type == "placement" } {
            createPlaceBlockage -type hard -box [list $blockage_x_tmp $blockage_y_tmp [expr "$blockage_x_tmp + $blockage_width_tmp"] [expr "$blockage_y_tmp + $blockage_height_tmp"] ]
        } else { #"routing"
            createRouteBlk -layer $blockage_layer -box [list $blockage_x_tmp $blockage_y_tmp [expr "$blockage_x_tmp + $blockage_width_tmp"] [expr "$blockage_y_tmp + $blockage_height_tmp"] ]
        }

        linfo [lobj "Blockage($blockage_width,$blockage_height) at (x,y) = ($blockage_x,$blockage_y) as '$blockage_type'"]
    }
}


# -- Debugging {unset}
dpop
