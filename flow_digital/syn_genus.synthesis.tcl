# ********************************************************************************
# File             : syn_genus.synthesis.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Synthesizes loaded design;
#                    Supports only single view
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Synthesis Execution"
set debug.object [lobj ${debug.title}]


# -- Generic
# convert verilog into generic logic gates (not tied to library)
ldebug [lobj "Generic Synthesis"]
syn_generic
report gates > [FILE_join "${fd_report}" "report.gates.syn_generic.txt"]

# -- Mapping
# convert generic logic gates into logic gates inside library (specified by .lib and .lef)
set debug.object [lobj ${debug.title} "Check if all required cells exist in PDK."]
syn_map
report gates     > [FILE_join "${fd_report}" "report.gates.syn_map.txt"]
report hierarchy > [FILE_join "${fd_report}" "report.hierarchy.syn_map.txt"]

# -- Optimization
# optimize synthesized netlist (power, timing, area, etc.)
ldebug [lobj "Optimized Synthesis"]
syn_opt

# -- Constants
# replace constants with tie cells
set debug.object [lobj ${debug.title} "Check 'cells/tie' in 'Technology Configuration'."]
if { ([llength ${technology.cells.tie.high}] >= 1) && ([llength ${technology.cells.tie.low}] >= 1) } {
    add_tieoffs -high ${technology.cells.tie.high} -low ${technology.cells.tie.low} -all -verbose
} else {
    add_tieoffs -all -verbose
}
report gates > [FILE_join "${fd_report}" "report.gates.syn_opt-physical.txt"]


# -- Debugging {unset}
dpop
