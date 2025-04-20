# ********************************************************************************
# File             : pnr_innovus.reports.tcl
# Version          : 1.2.3, Wed Apr 23 2025
# Description      : Extracts reports of placed design
#
# Flow Step        : pnr (Place-and-Route)
# EDA Tool         : Cadence Innovus
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added verification reporting
#     - v1.2.0: Added hold timing report
#     - v1.2.1: Added summary report & toggles
#     - v1.2.2: Separated net contribution in timing
#     - v1.2.3: Bug fix
# ********************************************************************************


# -- Debugging {set}
dpush
set debug.object [lobj "Reports Extraction"]


# -- Calculate timing
timeDesign -signoff
timeDesign -signoff -hold

# -- Summary
if {${steps.report.design}} {
    summaryReport -noHtml -outfile [FILE_join "${fd_report}" "report.summary.txt"]
}

# -- Power
if {${steps.report.power}} {
    #report_power > "${fd_report}/report.power.txt"
    set lis_period__ns [list 1 10 100 1e3]
    set lis_input_activity__fraction [list 0.50 0.10 0.01]
    foreach period__ns $lis_period__ns {
        foreach input_activity__fraction $lis_input_activity__fraction {
            set fdne_tmp [format [FILE_join "${fd_report}" "report.power.period-%gns.input_activity-%g.txt"] $period__ns $input_activity__fraction]
            
            set_power_analysis_mode -reset
            set_power_analysis_mode \
                -method static \
                -corner max \
                -off_pg_nets VDDau
            set_default_switching_activity -reset
            set_default_switching_activity -period $period__ns -input_activity $input_activity__fraction
            foreach {signal} ${design.general.constraints.clocks} {
                set name    [dict get $signal name]
                set primary [dict get $signal primary]

                if {$primary} {
                    set_switching_activity -activity $input_activity__fraction -net $name -period $period__ns
                } else {
                    set_switching_activity -activity $input_activity__fraction -net $name
                }
            }
            foreach {signal} ${design.general.constraints.resets} {
                set name [dict get $signal name]
                
                set_switching_activity -activity 0 -net $name
            }
            
            echo "reporting power to file: '$fdne_tmp'"
            report_power -net > $fdne_tmp
            
            # Note: in report_power... "Toggle" / 1e9 * period__ns = the activity of the net
        }
    }
}


if {${steps.report.routing}} {
    # -- Nets
    report_net > [FILE_join "${fd_report}" "report.net.txt"]

    # -- Routing
    report_route > [FILE_join "${fd_report}" "report.route.txt"]
    report_route -summary > [FILE_join "${fd_report}" "report.route.summary.txt"]

    # -- Via
    report_via > [FILE_join "${fd_report}" "report.via.txt"]
}

# -- Verify
if {${steps.report.rules}} {
    csource pnr_innovus.verify.tcl > [FILE_join "${fd_report}" "report.verify.txt"]
}

# -- Timing
if {${steps.report.timing}} {
    if { ${design.general.constraints.enable} } {
        set_db timing_report_unconstrained_paths false
    } else {
        set_db timing_report_unconstrained_paths true
    }

    set format [list \
        "instance" \
        "arc" \
        "edge" \
        "cell" \
        "instance_location" \
        "net" \
        "fanout" \
        "load" \
        "transition" \
        "delay" \
        "arrival"\
    ]

    if { ${design.general.constraints.enable} } {
        report_timing \
            -net \
            -format $format \
            -late \
            -nworst 1 \
            > [FILE_join "${fd_report}" "report.timing.setup.nworst-1.txt"]
        report_timing \
            -net \
            -format $format \
            -late \
            -nworst 1000 \
            > [FILE_join "${fd_report}" "report.timing.setup.nworst-1000.txt"]

        report_timing \
            -net \
            -format $format \
            -early \
            -nworst 1 \
            > [FILE_join "${fd_report}" "report.timing.hold.nworst-1.txt"]
        report_timing \
            -net \
            -format $format \
            -early \
            -nworst 1000 \
            > [FILE_join "${fd_report}" "report.timing.hold.nworst-1000.txt"]
    } else {
        report_timing \
            -net \
            -format $format \
            -nworst 1 \
            > [FILE_join "${fd_report}" "report.timing.nworst-1.txt"]
        report_timing \
            -net \
            -format $format \
            -nworst 1000 \
            > [FILE_join "${fd_report}" "report.timing.nworst-1000.txt"]
    }

}


# -- Debugging {unset}
dpop
