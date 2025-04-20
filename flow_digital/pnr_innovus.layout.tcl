# ********************************************************************************
# File             : pnr_innovus.layout.tcl
# Version          : 1.0.1, Mon Apr 14 2025
# Description      : Exports layout;
#                    Support LEF and GDS formats
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added minimum spacing in lef file
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.title "Layout Export"
set debug.object [lobj ${debug.title}]


set fdne_lef_out [FILE_join "${fd_export}" "${design.modules.top}.lef"]
write_lef_abstract $fdne_lef_out -5.8 -cutObsMinSpacing


set debug.object [lobj ${debug.title} "Check 'maps/@layers' in 'Technology Configuration'."]
set fdne_map "${technology.maps.layers}"
set gdsFileName [FILE_join "${fd_export}" "${design.modules.top}.gds"]
streamOut ${gdsFileName} -mapFile $fdne_map -libName DesignLib -units 1000 -mode ALL -dieAreaAsBoundary


# -- Debugging {unset}
dpop
