# ********************************************************************************
# File             : syn_genus.reports.tcl
# Version          : 1.0.3, Wed Apr 23 2025
# Description      : Extracts reports of synthesized design
#
# Flow Step        : syn (Synthesis)
# EDA Tool         : Cadence Genus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added Design Rules report
#     - v1.0.2: Added toggles
#     - v1.0.3: Improved feedback of timing analysis
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Reports Extraction"]


# -- Design
if {${steps.report.design}} {
    report gates     > [FILE_join "${fd_report}" "report.gates.syn.FINAL.txt"]
    report hierarchy > [FILE_join "${fd_report}" "report.hierarchy.syn.FINAL.txt"]
    report area      > [FILE_join "${fd_report}" "report.area.syn.FINAL.txt"]
    report clocks    > [FILE_join "${fd_report}" "report.clocks.syn.FINAL.txt"]
    report qor       > [FILE_join "${fd_report}" "report.qor.syn.FINAL.txt"]
}

# -- Power
if {${steps.report.power}} {
    report power -header           > [FILE_join "${fd_report}" "report.power.syn.FINAL.txt"]
    report power -by_libcell       > [FILE_join "${fd_report}" "report.power_by_libcell.syn.FINAL.txt"]
    report power -by_leaf_instance > [FILE_join "${fd_report}" "report.power_by_leaf_instance.syn.FINAL.txt"]
}

if {${steps.report.routing}} {
    # -- Nets
    report_nets > [FILE_join "${fd_report}" "report.nets.txt"]
    report_nets -cap_worst 100 > [FILE_join "${fd_report}" "report.nets.cap_worst-100.txt"]

    # -- Ports
    report_port [get_ports *] > [FILE_join "${fd_report}" "report.ports.txt"]

    # -- Parasitics (since qrcTechFile was loaded)
    write_parasitics -cap_unit "fF" -res_unit "ohm" > [FILE_join "${fd_report}" "${design.modules.top}.genus.spef"]
}

# -- Design Rules
if {${steps.report.rules}} {
    report_design_rules > [FILE_join "${fd_report}" "report.rules.txt"]
}

# -- Timing
if {${steps.report.timing}} {
    # Note: similar to "-unconstrained" flag in report_timing
    if { ${design.general.constraints.enable} } {
        set_db timing_report_unconstrained false
    } else {
        set_db timing_report_unconstrained true
    }

    set fields [list \
        "timing_point" \
        "flags" \
        "arc" \
        "edge" \
        "cell" \
        "fanout" \
        "load" \
        "transition" \
        "delay" \
        "arrival"
    ]

    report_timing \
        -fields $fields \
        -nworst 1 \
        > [FILE_join "${fd_report}" "report.timing.nworst-1.txt"]
    report_timing \
        -fields $fields \
        -nworst 1000 \
        > [FILE_join "${fd_report}" "report.timing.nworst-1000.txt"]
    report_timing -lint -verbose > [FILE_join "${fd_report}" "report.timing.analysis.txt"]
}


# -- Debugging {unset}
dpop
