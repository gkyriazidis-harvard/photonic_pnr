# ********************************************************************************
# File             : pnr_innovus.initialize.tcl
# Version          : 1.0.1, Sun Jun 2 2024
# Description      : Initializes synthesized design from VERILOG file;
#                    Reference in:
#                    <innovus-installation>/doc/innovusUG/Importing_and_Exporting_Designs.html
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added voltage attribute to 'technology/rails'
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Design Initialization"
set debug.object [lobj ${debug.title}]


# -- Node
set debug.object [lobj ${debug.title} "Check 'libraries/units' and 'details/node' in 'Technology Configuration'."]
set units ${technology.libraries.units}
set units_time [dict get $units time]
set units_cap  [dict get $units capacitance]
setLibraryUnit -time $units_time -cap $units_cap
if {![STRING_empty ${technology.details.node}]} {
    if { ${technology.physical.floorplan.scale} == "1.0" } {
        set node ${technology.details.node}
    } else {
        set node  [expr "${technology.details.node} / ${technology.physical.floorplan.scale}"]
    }
    setDesignMode -process ${technology.details.node}
}

# -- Load
# Note: 'init_*'' variables required by 'init_design' command
set debug.object [lobj ${debug.title} "Check 'modules/top' in 'Design Configuration'."]
set init_verilog [FILE_join "${fd_synthesis}" "export" "${design.modules.top}.syn.v"]
set init_lef_file ${instance.libraries.profiles}

set debug.object [lobj ${debug.title} "Check 'physical/floorplan/rails' in 'Technology Configuration'."]
foreach rail_tmp ${technology.physical.floorplan.rails} {
    set type_tmp [dict get $rail_tmp type]
    set signal_tmp [dict get $rail_tmp signal]

    if { $type_tmp == "ground" } {
        set init_gnd_net "${signal_tmp}"
    }
    if { $type_tmp == "power" } {
        set init_pwr_net "${signal_tmp}"
    }
}
if {![info exists init_pwr_net] || ![info exists init_gnd_net]} {
    throw {*} {"'power' or 'ground' net not specified."}
}

set debug.object [lobj ${debug.title} "Check 'modules/top' in 'Design Configuration'."]
set init_top_cell "${design.modules.top}"
set init_mmmc_file [FILE_join "." "flow_innovus.views.tcl"]

# -- Execution
set debug.object [lobj ${debug.title}]
init_design

# -- Supply nets
foreach rail_tmp ${technology.physical.floorplan.rails} {
    set signal [dict get $rail_tmp signal]
    #set type    [dict get $rail_tmp type]
    set voltage [dict get $rail_tmp voltage]
    
    if {![STRING_empty $voltage]} {
        set_pg_nets -net $signal -voltage $voltage
    }
    globalNetConnect $signal -type pgpin -pin $signal -inst * -all -override
    createPGPin $signal -net $signal
}

foreach supply_tmp ${design.general.supplies} {
    set name    [dict get $supply_tmp name]
    #set type    [dict get $supply_tmp type]
    set voltage [dict get $supply_tmp voltage]
    
    #if { $type == "power" } {
    #    dbSetIsNetPwr $name
    #} elseif { $type == "ground" } {
    #    dbSetIsNetGnd $name
    #}
    if {![STRING_empty $voltage]} {
        set_pg_nets -net $name -voltage $voltage
    }
    globalNetConnect $name -type pgpin -pin $name -inst * -all -override
    #createPGPin $name -net $name
}


# -- Debugging {unset}
dpop
