# ********************************************************************************
# File             : pnr_innovus.netlist.tcl
# Version          : 1.0.0, Fri Mar 22 2024
# Description      : Exports design as netlist
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Netlist Export"]


set fdne_netlist_pnr [FILE_join "${fd_export}" "${design.modules.top}.pnr.v"]
saveNetlist $fdne_netlist_pnr -lineLength 80

set lis_exclude [concat ${technology.cells.filler} ${technology.cells.tap}]
#set lis_exclude [concat $lis_exclude ${technology.cells.decap}]

# physical netlist for lvs (with & without power pin connections)
set fdne_netlist_pins [FILE_join "${fd_export}" "${design.modules.top}.phys.with_power_pins.v"]
saveNetlist $fdne_netlist_pins -lineLength 80 -phys -excludeLeafCell -includePowerGround -excludeCellInst $lis_exclude

set fdne_netlist_phys [FILE_join "${fd_export}" "${design.modules.top}.phys.no_power_pins.v"]
deletePGPin -all
clearGlobalNets
saveNetlist $fdne_netlist_phys -lineLength 80 -phys -excludeLeafCell -excludeCellInst $lis_exclude


# -- Debugging {unset}
dpop
