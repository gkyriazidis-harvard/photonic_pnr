sim:
    tclsh runme_vcs.tcl $(CMD) "$(FILELIST)" "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

wave:
    tclsh runme_verdi.tcl "$(PROJ)"

scan:
    tclsh runme_configs.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

syn:
    tclsh runme_genus.tcl batch syn_genus.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"
syn_interactive:
    tclsh runme_genus.tcl interactive "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

pnr:
    tclsh runme_innovus.tcl batch pnr_innovus.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"
pnr_interactive:
    tclsh runme_innovus.tcl interactive "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

pwr:
    tclsh runme_power.tcl $(CMD) "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

rundir:
    tclsh runme_rundir.tcl "$(TECH)" "$(MAP)" "$(PROJ)"

layout:
    tclsh runme_strmin.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

schematic:
    tclsh runme_ihdl.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

abstract:
    tclsh runme_abstract.tcl "$(TECH)" "$(DSGN)" "$(MAP)" "$(PROJ)"

drc:
    tclsh runme_drc.tcl "$(TECH)" "$(DSGN)" "$(PROJ)"

lvs:
    tclsh runme_lvs.tcl "$(TECH)" "$(DSGN)" "$(PROJ)"

summary:
    tclsh runme_summary.tcl "$(PROJ)"

virtuoso:
    tclsh runme_virtuoso.tcl "$(PROJ)"

tidy:
    rm -rf *~
    rm -rf .*~
    find . -name "*~" -delete
    find . -name ".*~" -delete

clean: tidy
    rm -rf .cadence
    rm -rf fv
    rm -rf csrc
    rm -rf lvs
    rm -rf drc
    rm -rf *.bin
    rm -rf *.log
    rm -rf .*.tstamp
    rm -rf *.rpt
    rm -rf *.key

nuke:
    { git ls-files -o -i -c --exclude-from=.gitignore & tclsh runme_nuke.tcl ${PROJ}; } | xargs rm -rf
    git clean -Xdf

