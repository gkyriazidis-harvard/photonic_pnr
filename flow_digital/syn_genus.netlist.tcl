# ********************************************************************************
# File             : syn_genus.netlist.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Creates netlist of synthesized design
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Netlist Creation"
set debug.object [lobj ${debug.title}]


# flatten netlist
ungroup -all -flatten
report hierarchy > [FILE_join "${fd_report}" "report.hierarchy.flatten.txt"]

# write synthesized netlist
write_hdl > [FILE_join "${fd_export}" "${design.modules.top}.syn.v"]


# -- Debugging {unset}
dpop
