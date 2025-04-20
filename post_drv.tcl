# -- Setup
set WG_WIDTH 1000
set CROSS_SIZE 20000
set BEND_SIZE  25000

set VIA_CELL   CRS12
set BEND_CELL  curve
set CROSS_CELL cross


# -- Load Design
set L [layout create ./export/Top.gds -dt_expand]
set TOP_MODULE [$L topcell]

$L cells
$L children $TOP_MODULE


# -- Detect Crosses
$L create cell $CROSS_CELL
$L create polygon $CROSS_CELL 2022 0 0 0 $CROSS_SIZE $CROSS_SIZE $CROSS_SIZE $CROSS_SIZE 0

$L create layer 9000
$L create layer 9001
$L AND 2021 2023 9000
$L NOT 9000 2022 9001 -hier 1
$L delete layer 9000
set clist [$L iterator poly $TOP_MODULE 9001 range 0 end]
foreach polygon $clist { 
    set x [lindex $polygon 0]
    set y [lindex $polygon 1]
    $L create ref $TOP_MODULE $CROSS_CELL [expr "$x+$WG_WIDTH/2-$CROSS_SIZE/2"] [expr "$y+$WG_WIDTH/2-$CROSS_SIZE/2"] 0 0 1
}
$L delete layer 9001


# -- Replace Vias
$L create cell $BEND_CELL
$L create polygon $BEND_CELL 2022 0 0 0 $BEND_SIZE $BEND_SIZE $BEND_SIZE $BEND_SIZE 0

#$L cellname $VIA_CELL $BEND_CELL
set clist [$L iterator sref $TOP_MODULE range 0 end -depth 0 0 -filterCell $VIA_CELL]
foreach cell $clist { 
    set ref [lindex $cell 0]
    set x [lindex $ref 1]
    set y [lindex $ref 2]
    
    set plist [$L query polygon $TOP_MODULE 0 0 $x $y $BEND_SIZE $BEND_SIZE -inside -list]
    set left 1
    set bottom 1
    #if {[llength $polys] == 2} {
        foreach seg $plist {
            set props [lindex $seg 0]
            set layer [lindex $props 0]

            if {$layer == 2021} {
                # Horizontal
                set distA [expr abs(min([lindex $props 1], [lindex $props 3], [lindex $props 5], [lindex $props 7])-$x)]
                set distB [expr abs(max([lindex $props 1], [lindex $props 3], [lindex $props 5], [lindex $props 7])-$x)]
                if {$distA > $distB} {
                    set left 1
                } else {
                    set left 0
                }
            } else {
                # Vertical
                set distA [expr abs(min([lindex $props 2], [lindex $props 4], [lindex $props 6], [lindex $props 8])-$y)]
                set distB [expr abs(max([lindex $props 2], [lindex $props 4], [lindex $props 6], [lindex $props 8])-$y)]
                if {$distA > $distB} {
                    set bottom 1
                } else {
                    set bottom 0
                }
            }
        }
    #}

    $L delete ref $TOP_MODULE $VIA_CELL $x $y 0 0 1
    if {$left && $bottom} {
        set angle  0
        set mirror 1
        set x_offset 0
        set y_offset 1
    } elseif {!$left && $bottom} {
        set angle  180
        set mirror 0
        set x_offset 2
        set y_offset 1
    } elseif {!$left && !$bottom} {
        set angle  180
        set mirror 1
        set x_offset 2
        set y_offset 1
    } else {
        set angle  0
        set mirror 0
        set x_offset 0
        set y_offset 1
    }
    $L create ref $TOP_MODULE $BEND_CELL [expr "$x+(2*$left-1)*$WG_WIDTH/2-$BEND_SIZE+$x_offset*$BEND_SIZE"] [expr "$y+(2*$bottom-1)*$WG_WIDTH/2-$BEND_SIZE+$y_offset*$BEND_SIZE"] $mirror $angle 1
}
$L delete cell $VIA_CELL


# -- Clear Overlaps
$L create layer 9021
$L create layer 9023
$L NOT 2021 2022 9021 -hier 1
$L NOT 2023 2022 9023 -hier 1

$L delete polygons $TOP_MODULE 2021
$L delete polygons $TOP_MODULE 2023
$L COPY 9021 2021
$L COPY 9023 2023
$L delete layer 9021
$L delete layer 9023


# -- Export Design
$L gdsout ./export/Top.gds
