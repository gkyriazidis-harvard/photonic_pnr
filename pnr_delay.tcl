set overhead 2.0

set_dont_use false [get_lib_cells delays/delay0p5]
set_dont_use false [get_lib_cells delays/delay1p0]
set_dont_use false [get_lib_cells delays/delay1p5]
set_dont_use false [get_lib_cells delays/delay2p0]
set_dont_use false [get_lib_cells delays/delay4p0]
set_dont_use false [get_lib_cells delays/delay8p0]
set_dont_use false [get_lib_cells delays/delay12p0]
set_dont_use false [get_lib_cells delays/delay16p0]

set cells [list \
    {16.0 "delay16p0"} \
    {12.0 "delay12p0"} \
    {8.0 "delay8p0"} \
    {4.0 "delay4p0"} \
    {2.0 "delay2p0"} \
    {1.5 "delay1p5"} \
    {1.0 "delay1p0"} \
    {0.5 "delay0p5"} \
]

set count 1
while {$count > 0} {
    set count 0

    report_timing -early -unique_pins -path_type end_slack_only -nworst 1000

    set collection [report_timing -early -unique_pins -path_type end_slack_only -nworst 1000 -collection]

    set lpins  [get_property $collection capturing_point_name]
    set lslack [get_property $collection slack]

    foreach pin $lpins slack $lslack {
        if {[expr {$slack < 0.0}]} {
            foreach cell $cells {
                set delay [lindex $cell 0]
                set name  [lindex $cell 1]

                if {[expr {-$slack > ($overhead+$delay)}]} {
                    if {![catch {ecoAddRepeater -net $pin -cell $name -logicalChangeOnly} errmsg]} {
                        incr count
                        break
                    }
                }
            }
        }
    }

    ecoPlace
    ecoRoute
}

routeDesign
optDesign -postRoute -drv
