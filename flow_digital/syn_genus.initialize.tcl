# ********************************************************************************
# File             : syn_genus.initialize.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Initializes design from HDL files;
#                    Supports Verilog and System-Verilog
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Design Initialization"
set debug.object [lobj ${debug.title}]


# -- Views
set debug.object [lobj ${debug.title} "Check 'libraries' in 'Technology or Mapping Configuration'."]
if {${mapping.general.views.enable}} {
    # read libs, qrc
    read_mmmc [FILE_join "." "syn_genus.views.tcl"]
    
    # read lefs
    read_physical -lefs ${instance.libraries.profiles}
} else {
    # read power/timing libs
    read_libs ${instance.libraries.cells}
}

# -- Design
set debug.object [lobj ${debug.title} "Check 'sources' in 'Design Configuration'."]
foreach src ${design.sources} {
    read_hdl -language sv $src
}
# Note: creates directory 'fv'
set debug.object [lobj ${debug.title} "Check 'modules/top' in 'Design Configuration'."]
elaborate "${design.modules.top}"

# -- Execution
set debug.object [lobj ${debug.title}]
init_design


# -- Debugging {unset}
dpop
