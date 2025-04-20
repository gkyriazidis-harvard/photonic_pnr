# ********************************************************************************
# File             : pnr_innovus.pins.tcl
# Version          : 1.0.3, Wed Apr 23 2025
# Description      : Places manually-defined pins
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added support for partial selection
#     - v1.0.2: Fixed pin name bug
#     - v1.0.3: Changed empty pins configuration
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Pins Placement" "Check 'physical/floorplan/pins' in 'Mapping Configuration'."]


if { ${mapping.physical.floorplan.pins.enable} } {
    set margins ${instance.physical.dimensions.margins}
    
    set dims_width  ${instance.physical.dimensions.width}
    set dims_height ${instance.physical.dimensions.height}
    set margins ${instance.physical.dimensions.margins}
    set margin_horizontal [dict get $margins horizontal]
    set margin_vertical   [dict get $margins vertical]
    set margin_left   $margin_horizontal
    set margin_right  $margin_horizontal
    set margin_top    $margin_vertical
    set margin_bottom $margin_vertical

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
    
    set pins ${mapping.physical.floorplan.pins}
    set pin_layer  [dict get $pins layer]
    set pin_width  [dict get $pins width]
    set pin_height [dict get $pins height]

    setPlaceMode -place_global_place_io_pins true
    setPinAssignMode -pinEditInBatch true
    if {[llength ${mapping.physical.floorplan.pins.all}] == 0} {
        set instance.physical.floorplan.pins.top    []
        set instance.physical.floorplan.pins.bottom []
        set instance.physical.floorplan.pins.left   []
        set instance.physical.floorplan.pins.right  []
        set instance.physical.floorplan.pins.fixed  []
    } else {
        set instance.physical.floorplan.pins.top    ${mapping.physical.floorplan.pins.top}
        set instance.physical.floorplan.pins.bottom ${mapping.physical.floorplan.pins.bottom}
        set instance.physical.floorplan.pins.left   ${mapping.physical.floorplan.pins.left}
        set instance.physical.floorplan.pins.right  ${mapping.physical.floorplan.pins.right}
        set instance.physical.floorplan.pins.fixed  ${mapping.physical.floorplan.pins.fixed}
    }
    for { set i 0}  {$i < 4} {incr i} {
        switch $i {
            0 {
                set pins ${instance.physical.floorplan.pins.top}
                set side  TOP
            }
            1 {
                set pins ${instance.physical.floorplan.pins.bottom}
                set side  BOTTOM
            }
            2 {
                set pins ${instance.physical.floorplan.pins.left}
                set side  LEFT
            }
            3 {
                set pins ${instance.physical.floorplan.pins.right}
                set side  RIGHT
            }
        }
        set groups []
        if {[llength $pins] > 0} {
            foreach pin $pins {
                set signal [dict get $pin signal]
                set layer  [dict get $pin layer]
                set width  [dict get $pin width]
                set height [dict get $pin height]

                set names [PINS_expand $signal]

                if { [dict exists $groups $layer signals] } {
                    set tmp [dict get $groups $layer signals]
                    dict set groups $layer signals [lappend tmp {*}$names]
                } else {
                    dict set groups $layer signals [list {*}$names]
                }
                if { [dict exists $groups $layer width] } {
                    set width_prev [dict get $groups $layer width]
                } else {
                    set width_prev ""
                }
                if { [dict exists $groups $layer height] } {
                    set height_prev [dict get $groups $layer height]
                } else {
                    set height_prev ""
                }
                
                if { $width_prev == "" } {
                    dict set groups $layer width $width
                } elseif { $width != "" } {
                    dict set groups $layer width  [expr "max($width,$width_prev)"]
                }
                if { $height_prev == "" } {
                    dict set groups $layer height $height
                } elseif { $height != "" } {
                    dict set groups $layer height [expr "max($height,$height_prev)"]
                }
            }
            
            dict for {layer group} $groups {
                set signals [dict get $group signals]
                set width   [dict get $group width]
                set height  [dict get $group height]
                
                set names [get_property [get_ports "$signals"] "full_name"]
                switch $side {
                    TOP {
                        set start [list $x_start $y_high]
                        set end   [list $x_end   $y_high]
                    }
                    BOTTOM {
                        set start [list $x_end   $y_low]
                        set end   [list $x_start $y_low]
                    }
                    LEFT   {
                        set start [list $x_low $y_start]
                        set end   [list $x_low $y_end  ]
                    }
                    RIGHT  {
                        set start [list $x_high $y_end  ]
                        set end   [list $x_high $y_start]
                    }
                }
                if {[llength $names] == 1} {
                    set spread CENTER
                } else {
                    set spread RANGE
                }

                # Note: apply scale
                if { $width == "" } {
                    set pin_width_tmp $pin_width
                } else {
                    set pin_width_tmp $width
                }
                if { $height == "" } {
                    set pin_height_tmp $pin_height
                } else {
                    set pin_height_tmp $height
                }
                if { ${technology.physical.floorplan.scale} == "1.0" } {
                    #set pin_width_tmp  $pin_width
                    #set pin_height_tmp $pin_height
                    set start_tmp $start
                    set end_tmp   $end
                } else {
                    set pin_width_tmp  [expr "$pin_width_tmp / ${technology.physical.floorplan.scale}"]
                    set pin_height_tmp [expr "$pin_height_tmp / ${technology.physical.floorplan.scale}"]
                    set start_tmp [LIST_map $start "%x / ${technology.physical.floorplan.scale}"]
                    set end_tmp   [LIST_map $end   "%x / ${technology.physical.floorplan.scale}"]
                }
                
                if { $layer == "" } {
                    set pin_layer_tmp $pin_layer
                } else {
                    set pin_layer_tmp $layer
                }
                #editPin \
                #    -pin $names -spreadType $spread \
                #    -layer $pin_layer_tmp -pinWidth $pin_width -pinDepth $pin_height \
                #    -side $side -start $start -end $end -fixedPin 1
                editPin \
                    -pin $names -spreadType $spread \
                    -layer $pin_layer_tmp -pinWidth $pin_height_tmp -pinDepth $pin_width_tmp \
                    -side $side -start $start_tmp -end $end_tmp -fixedPin 1
            }
        }
    }

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

    set pins ${instance.physical.floorplan.pins.fixed}
    foreach {pin} $pins {
        set signal [dict get $pin signal]
        set width  [dict get $pin width]
        set height [dict get $pin height]
        set pos_x  [dict get $pin x]
        set pos_y  [dict get $pin y]

        set inst_layer [dict get $pin layer]
        if {[STRING_empty $inst_layer]} {
            set inst_layer $pin_layer
        }

        set signal [get_property [get_ports $signal] "full_name"]

        set pos_x [expr "[VARS_apply $pos_x $dims]"]
        set pos_y [expr "[VARS_apply $pos_y $dims]"]

        # Note: apply scale
        if { $width == "" } {
            set pin_width_tmp $pin_width
        } else {
            set pin_width_tmp $width
        }
        if { $height == "" } {
            set pin_height_tmp $pin_height
        } else {
            set pin_height_tmp $height
        }
        set pos_x_tmp $pos_x
        set pos_y_tmp $pos_y
        if { ${technology.physical.floorplan.scale} != "1.0" } {
            set pin_width_tmp  [expr "$pin_width_tmp / ${technology.physical.floorplan.scale}"]
            set pin_height_tmp [expr "$pin_height_tmp / ${technology.physical.floorplan.scale}"]
            set pos_x_tmp [expr "$pos_x_tmp / ${technology.physical.floorplan.scale}"]
            set pos_y_tmp [expr "$pos_y_tmp / ${technology.physical.floorplan.scale}"]
        }
        #editPin \
        #    -pin $signal \
        #    -layer $inst_layer -pinWidth $pin_width -pinDepth $pin_height \
        #    -global_location -side INSIDE -assign [list $pos_x $pos_y] -fixedPin 1
        editPin \
            -pin $signal \
            -layer $inst_layer -pinWidth $pin_height_tmp -pinDepth $pin_width_tmp \
            -global_location -side INSIDE -assign [list $pos_x_tmp $pos_y_tmp] -fixedPin 1 -fixOverlap 0
    }
    setPinAssignMode -pinEditInBatch false
    
    if {![STRING_empty ${mapping.physical.floorplan.pins.source}]} {
        source "${mapping.physical.floorplan.pins.source}"
    }

    if { ${mapping.physical.floorplan.pins.skip} } {
        set items [dbGet [dbGet top.terms {.pStatus == "unplaced"}].name]
        foreach item $items {
            setAttribute -net $item -skip_routing true
        }
    } else {
        set items [list ]
    }
    set instance.physical.floorplan.pins.skip.enable ${mapping.physical.floorplan.pins.skip}
    set instance.physical.floorplan.pins.skip.items  $items
} else {
    set instance.physical.floorplan.pins.skip.enable false
    set instance.physical.floorplan.pins.skip.items  [list ]
}


# -- Debugging {unset}
dpop
