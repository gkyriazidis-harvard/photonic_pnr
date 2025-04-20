# ********************************************************************************
# File             : pnr_innovus.floorplan.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Creates floorplan
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Floorplan Creation"
set debug.object [lobj ${debug.title}]


# -- Pre-load floorplan
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/initial' in 'Mapping Configuration'."]
if {![STRING_empty "${mapping.physical.floorplan.initial}"]} {
    defIn "${mapping.physical.floorplan.initial}"
}

# -- Set floorplan box to default
set fp_box [lindex [dbGet top.fPlan.box] 0]
set fp_llx [lindex $fp_box 0]
set fp_lly [lindex $fp_box 1]
set fp_urx [lindex $fp_box 2]
set fp_ury [lindex $fp_box 3]

# -- Set scales
set debug.object [lobj ${debug.title} "Check 'physical/floorplan/scale' in 'Technology Configuration'."]
if { ${technology.physical.floorplan.scale} != "1.0" } {
    set fp_llx [expr "$fp_llx * ${technology.physical.floorplan.scale}"]
    set fp_lly [expr "$fp_lly * ${technology.physical.floorplan.scale}"]
    set fp_urx [expr "$fp_urx * ${technology.physical.floorplan.scale}"]
    set fp_ury [expr "$fp_ury * ${technology.physical.floorplan.scale}"]
}

# -- Customize floorplan box
set fp_area [expr "($fp_urx - $fp_llx) * ($fp_ury - $fp_lly)"]

set width  ${mapping.physical.dimensions.width}
set height ${mapping.physical.dimensions.height}

if {[STRING_empty $width] & ![STRING_empty $height]} {
    linfo [lobj "Width not specified; width = area / height."]
    set width [expr "${fp_area} / ${height}"]
} elseif {![STRING_empty $width] & [STRING_empty $height]} {
    linfo [lobj "Height not specified; height = area / width."]
    set height [expr "${fp_area} / ${width}"]
} elseif {[STRING_empty $width] & [STRING_empty $height]} {
    linfo [lobj "Height and Width not specified; using defaults."]
    set width [expr "$fp_urx - $fp_llx"]
    set height [expr "$fp_ury - $fp_lly"]
}

set debug.object [lobj ${debug.title} "Check 'physical/floorplan/track' in 'Technology Configuration'."]
set track ${technology.physical.floorplan.track}

set debug.object [lobj ${debug.title} "Check 'physical/dimensions/margins' in 'Mapping Configuration'."]
set margins ${mapping.physical.dimensions.margins}
set margin_horizontal [dict get $margins horizontal]
if {[STRING_empty $margin_horizontal]} {
    set margin_horizontal [expr "2 * $track"]
}
set margin_vertical [dict get $margins vertical]
if {[STRING_empty $margin_vertical]} {
    set margin_vertical [expr "2 * $track"]
}
set margin_top    $margin_vertical
set margin_bottom $margin_vertical
set margin_left   $margin_horizontal
set margin_right  $margin_horizontal

dict set margins horizontal $margin_horizontal
dict set margins vertical   $margin_vertical


set debug.object [lobj ${debug.title} "Check 'physical/floorplan/site' in 'Technology Configuration'."]
set site ${technology.physical.floorplan.site}
# Note: apply scale
if { ${technology.physical.floorplan.scale} == "1.0" } {
    set width_tmp  $width
    set height_tmp $height
    set margin_left_tmp   $margin_left
    set margin_bottom_tmp $margin_bottom
    set margin_right_tmp  $margin_right
    set margin_top_tmp    $margin_top
} else {
    set width_tmp  [expr "$width / ${technology.physical.floorplan.scale}"]
    set height_tmp [expr "$height / ${technology.physical.floorplan.scale}"]
    set margin_left_tmp   [expr "$margin_left / ${technology.physical.floorplan.scale}"]
    set margin_bottom_tmp [expr "$margin_bottom / ${technology.physical.floorplan.scale}"]
    set margin_right_tmp  [expr "$margin_right / ${technology.physical.floorplan.scale}"]
    set margin_top_tmp    [expr "$margin_top / ${technology.physical.floorplan.scale}"]
}
floorPlan -site $site -s $width_tmp $height_tmp $margin_left_tmp $margin_bottom_tmp $margin_right_tmp $margin_top_tmp -noSnapToGrid -flip "s"

# Note: store for next steps
set instance.physical.dimensions.width   $width
set instance.physical.dimensions.height  $height
set instance.physical.dimensions.margins $margins

linfo [lobj "(Width,Height) = ($width,$height); Margins = {$margins}"]


# -- Debugging {unset}
dpop
