# Flow for syn/pnr

RUNDIR="flow_digital"
BASE=".."

all: sim_rtl syn sim_syn pwr_syn pnr sim_pnr pwr_pnr release
analysis: sim_rtl sim_syn sim_pnr pwr_pnr
build: syn pnr release

check:
ifndef TECH
override TECH = "technology_configuration.xml"
endif
ifndef DSGN
override DSGN = "design_configuration.xml"
endif
ifndef MAP
override MAP = "mapping_configuration.xml"
endif
ifndef PROJ
override PROJ = "project_configuration.xml"
endif
override BASE = $(shell realpath --relative-to="$(RUNDIR)" ".")

sim_rtl: check
    $(MAKE) sim CMD=RTL TECH="$(TECH)" DSGN="$(DSGN)" MAP="$(MAP)" PROJ="$(PROJ)"
sim_syn: check
    $(MAKE) sim CMD=SYN TECH="$(TECH)" DSGN="$(DSGN)" MAP="$(MAP)" PROJ="$(PROJ)"
sim_pnr: check
    $(MAKE) sim CMD=PNR TECH="$(TECH)" DSGN="$(DSGN)" MAP="$(MAP)" PROJ="$(PROJ)"
sim:
    (cd $(RUNDIR); make sim CMD="$(CMD)" TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

wave: check
    (cd $(RUNDIR); make wave PROJ="$(BASE)/$(PROJ)";)

scan: check
    (cd $(RUNDIR); make scan TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

syn: check
    (cd $(RUNDIR); make syn TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)
syn_interactive:
    (cd $(RUNDIR); make syn_interactive TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

pnr: check
    (cd $(RUNDIR); make pnr TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)
pnr_interactive:
    (cd $(RUNDIR); make pnr_interactive TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

pre: check
    tclsh pre_fanout.tcl "$(DSGN)" "$(PROJ)"

post: check
    tclsh runme_drv.tcl batch post_drv.tcl "$(DSGN)" "$(PROJ)"
post_interactive:
    tclsh runme_drv.tcl interactive "$(DSGN)" "$(PROJ)"

rundir: check
    (cd $(RUNDIR); make rundir TECH="$(TECH)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

layout: check
    (cd $(RUNDIR); make layout TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

schematic: check
    (cd $(RUNDIR); make schematic TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

abstract: check
    (cd $(RUNDIR); make abstract TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" MAP="$(BASE)/$(MAP)" PROJ="$(BASE)/$(PROJ)";)

release: check
    $(MAKE) rundir    TECH="$(TECH)" MAP="$(MAP)" PROJ="$(PROJ)"
    $(MAKE) layout    TECH="$(TECH)" DSGN="$(DSGN)" MAP="$(MAP)" PROJ="$(PROJ)"
    $(MAKE) schematic TECH="$(TECH)" DSGN="$(DSGN)" MAP="$(MAP)" PROJ="$(PROJ)"
    tclsh runme_rundir.tcl "$(TECH)" "$(MAP)" "$(PROJ)"

drc: check
    (cd $(RUNDIR); make drc TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" PROJ="$(BASE)/$(PROJ)";)

lvs: check
    (cd $(RUNDIR); make lvs TECH="$(TECH)" DSGN="$(BASE)/$(DSGN)" PROJ="$(BASE)/$(PROJ)";)

summary: check
    (cd $(RUNDIR); make summary PROJ="$(BASE)/$(PROJ)";)

virtuoso: check
    tclsh runme_virtuoso.tcl "$(PROJ)"

tidy:
    (cd $(RUNDIR); make tidy)

clean: tidy
    (cd $(RUNDIR); make clean)

nuke: check tidy clean
    (cd $(RUNDIR); make nuke PROJ="$(BASE)/$(PROJ)";)
