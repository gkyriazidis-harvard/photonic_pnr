# ********************************************************************************
# File             : pnr_innovus.mesh.tcl
# Version          : 1.1.2, Sat Jun 8 2024
# Description      : Forms Power Mesh;
#                    Supports Rings, Rails & Grid
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Changed structure of 'grid' group
#     - v1.1.1: Added toggle for rails
#     - v1.1.2: Added extra 'signals' option in 'floorplan/grip/stripes'
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Power Mesh Formation"
set debug.object [lobj ${debug.title}]


#setViaGenMode -viarule_preference
#setViaGenMode -allow_wire_shape_change false

# -- Rings
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/rings' in 'Mapping Configuration'."]
if { ${mapping.physical.floorplan.rings.enable} } {
    ldebug [lobj "Rings Formation"]
    
    set track ${technology.physical.floorplan.track}
    
    set nets ${mapping.physical.floorplan.rings.signals}    

    set rings_width   ${mapping.physical.floorplan.rings.width}
    set rings_spacing ${mapping.physical.floorplan.rings.spacing}
    
    set horizontal_layer [dict get ${mapping.physical.floorplan.rings.stripes.horizontal} layer]
    set vertical_layer   [dict get ${mapping.physical.floorplan.rings.stripes.vertical}   layer]
    set horizontal_width [dict get ${mapping.physical.floorplan.rings.stripes.horizontal} width]
    set vertical_width   [dict get ${mapping.physical.floorplan.rings.stripes.vertical}   width]
    set horizontal_spacing [dict get ${mapping.physical.floorplan.rings.stripes.horizontal} spacing]
    set vertical_spacing   [dict get ${mapping.physical.floorplan.rings.stripes.vertical}   spacing]
    

    set layers []
    dict set layers top    $horizontal_layer
    dict set layers bottom $horizontal_layer
    dict set layers left   $vertical_layer
    dict set layers right  $vertical_layer


    if { [STRING_empty $horizontal_width] } {
        set horizontal_width $rings_width
    }
    if { [STRING_empty $vertical_width] } {
        set vertical_width $rings_width
    }
    if { [STRING_empty $horizontal_spacing] } {
        set horizontal_spacing $rings_spacing
    }
    if { [STRING_empty $vertical_spacing] } {
        set vertical_spacing $rings_spacing
    }
    # Note: apply scale
    if { ${technology.physical.floorplan.scale} == "1.0" } {
        set horizontal_width_tmp $horizontal_width
        set vertical_width_tmp   $vertical_width

        set horizontal_spacing_tmp $horizontal_spacing
        set vertical_spacing_tmp   $vertical_spacing
    } else {
        set horizontal_width_tmp [expr "$horizontal_width / ${technology.physical.floorplan.scale}"]
        set vertical_width_tmp   [expr "$vertical_width / ${technology.physical.floorplan.scale}"]
        
        set horizontal_spacing_tmp [expr "$horizontal_spacing / ${technology.physical.floorplan.scale}"]
        set vertical_spacing_tmp   [expr "$vertical_spacing / ${technology.physical.floorplan.scale}"]
    }

    set widths []
    dict set widths top    $horizontal_width_tmp
    dict set widths bottom $horizontal_width_tmp
    dict set widths left   $vertical_width_tmp
    dict set widths right  $vertical_width_tmp

    set spacings []
    dict set spacings top    $horizontal_spacing_tmp
    dict set spacings bottom $horizontal_spacing_tmp
    dict set spacings left   $vertical_spacing_tmp
    dict set spacings right  $vertical_spacing_tmp


    set offset_center     [dict get ${mapping.physical.floorplan.rings.offset} center]
    set offset_horizontal [dict get ${mapping.physical.floorplan.rings.offset} horizontal]
    set offset_vertical   [dict get ${mapping.physical.floorplan.rings.offset} vertical]
    puts $offset_vertical
    if { $offset_center } {
        addRing \
            -nets $nets \
            -type core_rings -center 1 \
            -layer $layers -width $widths -spacing $spacings
    } else {
        addRing \
            -nets $nets \
            -type core_rings -center 0 -offset "top $offset_vertical bottom $offset_vertical left $offset_horizontal right $offset_horizontal" \
            -layer $layers -width $widths -spacing $spacings
    }
}

# -- Rails
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/rails' in 'Technology Configuration'."]
if { ${mapping.physical.floorplan.rails.enable} } {
    ldebug [lobj "Rail Formation"]

    set rails ${technology.physical.floorplan.rails}
    set rail_width [dict get [lindex $rails 0] width]

    set dims_width  ${instance.physical.dimensions.width}
    set dims_height ${instance.physical.dimensions.height}
    set margins ${instance.physical.dimensions.margins}
    set margin_horizontal [dict get $margins horizontal]
    set margin_vertical   [dict get $margins vertical]
    set margin_left $margin_vertical
    set margin_bottom $margin_vertical
    puts ${technology.physical.floorplan.rails}

    set rail_pitch [expr "2*$track"]
    set index 0;    
    foreach {rail} ${technology.physical.floorplan.rails} {
        set signal [dict get $rail signal]
        set width  [dict get $rail width]
        set layer  [dict get $rail layer]
        
        set start [expr "$margin_bottom - 0.5 * $rail_width + $index*$track"]
        set stop  [expr "$margin_bottom + $dims_height + 0.5 * $rail_width"]    
        
        # Note: apply scale
        if { ${technology.physical.floorplan.scale} == "1.0" } {
            set width_tmp      $width
            set rail_pitch_tmp $rail_pitch
            set start_tmp      $start
            set stop_tmp       $stop
        } else {
            set width_tmp      [expr "$width / ${technology.physical.floorplan.scale}"]
            set rail_pitch_tmp [expr "$rail_pitch / ${technology.physical.floorplan.scale}"]
            set start_tmp      [expr "$start / ${technology.physical.floorplan.scale}"]
            set stop_tmp       [expr "$stop / ${technology.physical.floorplan.scale}"]
        }
        
        #addStripe \
        #    -direction horizontal -layer $layer \
        #    -nets [list $signal] \
        #    -width $width -set_to_set_distance $rail_pitch \
        #    -start $start -stop $stop
        setAddStripeMode -allow_jog none
        addStripe \
            -direction horizontal -layer $layer \
            -nets [list $signal] -switch_layer_over_obs 0 \
            -width $width_tmp -set_to_set_distance $rail_pitch_tmp \
            -start $start_tmp -stop $stop_tmp

        set index [expr "$index+1"]
    }
}

# -- Grid
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/grid' in 'Mapping Configuration'."]
if { ${mapping.physical.floorplan.grid.enable} } {
    ldebug [lobj "Grid Formation"]

    set grid_signals ${mapping.physical.floorplan.grid.signals}
    set grid_width   ${mapping.physical.floorplan.grid.width}
    set grid_spacing ${mapping.physical.floorplan.grid.spacing}
    set grid_pitch   ${mapping.physical.floorplan.grid.pitch}
    
    foreach {stripe} ${mapping.physical.floorplan.grid.stripes} {
        set signals   [dict get $stripe signals]
        set layer     [dict get $stripe layer]
        set direction [dict get $stripe direction]
        set width     [dict get $stripe width]
        set spacing   [dict get $stripe spacing]
        set pitch     [dict get $stripe pitch]
        set start     [dict get $stripe start]
        set stop      [dict get $stripe stop]
        set keepout   [dict get $stripe keepout]
        
        if { [llength $signals] == 0 } {
            set stripe_signals $signals
        } else {
            set stripe_signals $grid_signals
        }
        if { [STRING_empty $width] } {
            set stripe_width $grid_width
        } else {
            set stripe_width $width
        }
        if { [STRING_empty $spacing] } {
            set stripe_spacing $grid_spacing
        } else {
            set stripe_spacing $spacing
        }
        if { [STRING_empty $pitch] } {
            if { [STRING_empty $grid_pitch] } {
                set stripe_pitch [expr "$stripe_width+$stripe_spacing"]
            } else {
                set stripe_pitch $grid_pitch
            }
        } else {
            set stripe_pitch $pitch
        }
        
        if { [STRING_empty $start] || [STRING_empty $stop] } {
            if { $direction == "horizontal" } {
                set start [expr "$margin_bottom - 0.5 * $stripe_width"]
                set stop  [expr "$margin_bottom + $dims_height + 0.5 * $stripe_width"]
                set dir1  "top"
                set dir2  "bottom"
            } else {
                set start [expr "$margin_left - 0.5 * $stripe_width"]
                set stop  [expr "$margin_left + $dims_width + 0.5 * $stripe_width"]
                set dir1  "left"
                set dir2  "right"
            }
        }
        
        # Note: apply scale
        if { ${technology.physical.floorplan.scale} == "1.0" } {
            set stripe_width_tmp   $stripe_width
            set stripe_spacing_tmp $stripe_spacing
            set stripe_pitch_tmp   $stripe_pitch
            set start_tmp          $start
            set stop_tmp           $stop
            set keepout_tmp        $keepout
        } else {
            set stripe_width_tmp   [expr "$stripe_width / ${technology.physical.floorplan.scale}"]
            set stripe_spacing_tmp [expr "$stripe_spacing / ${technology.physical.floorplan.scale}"]
            set stripe_pitch_tmp   [expr "$stripe_pitch / ${technology.physical.floorplan.scale}"]
            set start_tmp          [expr "$start / ${technology.physical.floorplan.scale}"]
            set stop_tmp           [expr "$stop / ${technology.physical.floorplan.scale}"]
            set keepout_tmp        [expr "$keepout / ${technology.physical.floorplan.scale}"]
        }

        set signals [string map {"," " "} $signals]
        if {[llength $stripe_signals] > 1} {
            addStripe \
                -direction $direction -layer $layer \
                -nets $stripe_signals \
                -width $stripe_width_tmp -spacing $stripe_spacing_tmp -set_to_set_distance $stripe_pitch_tmp \
                -start $start_tmp -stop $stop_tmp
        } else {
            addStripe \
                -direction $direction -layer $layer \
                -nets $stripe_signals \
                -width $stripe_width_tmp -set_to_set_distance $stripe_pitch_tmp \
                -start $start_tmp -stop $stop_tmp
            
        }
        
        if {![STRING_empty $keepout]} {
            specify_pg_keepout -cell * -layer $layer -$dir1 $keepout_tmp -$dir2 $keepout_tmp
        }
    }
}


# -- Debugging {unset}
dpop
