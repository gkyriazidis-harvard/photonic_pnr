# ********************************************************************************
# File             : pwr_genus.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Initializes implemented design from HDL files
#
# Flow Step        : pwr (Power Consumption Calculation)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Design Initialization"
set debug.object [lobj ${debug.title}]


set instance.command ${command}

# -- Views
set debug.object [lobj ${debug.title} "Check 'libraries' in 'Technology or Mapping Configuration'."]
if {${mapping.general.views.enable}} {
    # read libs, qrc
    read_mmmc [FILE_join "." "pwr_genus.views.tcl"]
    
    # read lefs
    read_physical -lefs ${instance.libraries.profiles}
} else {
    # read power/timing libs
    read_libs ${instance.libraries.cells}
}

# -- Design
set debug.object [lobj ${debug.title} "Check if implemented design exists."]
if { ${instance.command} == "SYN" } {
    read_hdl -language sv [FILE_join "${fd_synthesis}" "export" "${design.modules.top}.syn.v"]
} else { # ${instance.command} == "PNR"
    read_hdl -language sv [FILE_join "${fd_physical}" "export" "${design.modules.top}.pnr.v"]
}
# Note: creates directory 'fv'
elaborate "${design.modules.top}"

# -- Execution
set debug.object [lobj ${debug.title}]
init_design


# -- Debugging {unset}
dpop
