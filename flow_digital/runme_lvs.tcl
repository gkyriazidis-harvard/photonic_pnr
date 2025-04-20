#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set design_configuration     [lindex $argv 1]
set project_configuration    [lindex $argv 2]

set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
set library_name [XML_readEntry "/root/project/digital/library/name" $project_configuration]
set fd_virtuoso [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]
set fd_output [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='lvs'\]/path" \
$project_configuration]
set fdne_rul_template [XML_readExternals "/root/pdk/variables" "/root/pdk/rules/rule\[@type='lvs'\]/" [list \
    "path" \
    "file" \
] "/" $technology_configuration]

if { [file exists "$fd_output"] } {
    exec rm -rf "$fd_output"
}
exec mkdir -p "$fd_output"

# create lvs deck from template
exec sed -e "s/\${top}/${top}/g" "$fdne_rul_template" > "${fd_output}/_lvsRules.lvs.${top}.lvs_"

# remove existing gds
if { [file exists "${fd_output}/${top}.calibre.db"] } {
    echo "removing existing gds!!!"
    exec rm "${fd_output}/${top}.calibre.db"
}

set fdne_layerMap [XML_readExternals "/root/pdk/variables" "/root/pdk/maps/map\[@type='layers-virtuoso'\]/" [list \
    "path" \
    "file" \
] "/" $technology_configuration]

set prevdir "[pwd]"
#set fd_temp [exec realpath --relative-to="${fd_virtuoso}" "${fd_output}"]
set fd_temp [exec realpath "${fd_output}"]
#set fdne_template [exec realpath --relative-to="${fd_virtuoso}" "si.template.env"]
set fdne_template [exec realpath "si.template.env"]
cd "${fd_virtuoso}"
exec strmout \
    -library     "${library_name}" \
    -strmFile    "${top}.calibre.db" \
    -topCell     "${top}" \
    -view        "layout" \
    -runDir      "${fd_temp}" \
    -logFile     "PIPO.LOG.${top}" \
    -summaryFile "PIPO.SUM.${top}" \
    -layerMap    "${fdne_layerMap}" \
    -case        "Preserve" \
    -convertDot  "node" \
>&@stdout

#if virtuoso doesn't run, si throws error
if { ! [file exists "AUXLIB/.oalib"] } {
    exec echo "exit" | virtuoso -nographE
}

# remove existing files (this is done when running through Calibre Interactive LVS GUI, see Transcript after running cell twice)
# remove PHDB directory and .extf file (Calibre does this when running through GUI)
if { [file exists "svdb/${top}.phdb"] } {
    # Persistent Hierarchical DataBase
    echo "removing existing PHDB directory!!!"
}
if { [file exists "svdb/${top}.xdb"] } {
    echo "removing existing XDB directory!!!"
}
if { [file exists "svdb/${top}.dv"] } {
    echo "removing existing .dv file!!!"
}
if { [file exists "svdb/${top}.extf"] } {
    echo "removing existing .extf file!!!"
}
if { [file exists "svdb/${top}.lvsf"] } {
    echo "removing existing .lvsf file!!!"
}
exec rm -rf svdb

# calibre -spice dumps output files, so cd to $fd_output first

# create template
set fdne_si_template_env [exec readlink -f "${fdne_template}"]
exec sed -e "s/\${top}/${top}/g" \
         -e "s/\${lib}/${library_name}/g" \
         $fdne_si_template_env \
> "si.env"

# run si to extract netlist from schematic viewer
#[novels4]: ~> si -help
#Virtuoso Framework License (111) was checked out successfully. Total checkout time was 0.05s.
#Usage: si [run directory] [-batch [-command commandName]]
#          [-difftest] [-noenv] [-cdslib path]
#
# si will look for si.env in the run_directory
# si [run directory] -batch -command netlist
exec si -batch -command netlist >&@stdout
#cd ../output_lvs
#exec si -batch -command netlist -cdslib ../output_virtuoso/cds.lib

exec mv "${top}.src.net" "${fd_temp}/."
cd "$prevdir"
set prevdir ""

#$ echo $MGC_HOME
#/opt/mentor/calibre/2018.1_36.27

# run LVS
# copied from Transcript in Calibre Interactive LVS GUI
cd "${fd_output}"
set fdne_rul [exec readlink -f _lvsRules.lvs.${top}.lvs_]
exec calibre \
    -spice "${top}.lvs.sp" \
    -lvs \
    -hier \
    -nowait \
    -turbo -turbo_all \
    $fdne_rul \
    | tee "runme_lvs.calibre.tee.log" \
>&@stdout

# 1 line summary (extract line with CORRECT/INCORRECT as the 2nd line after the one with "Result")
exec grep "Result" -A 2 "${top}.lvs.report" | tail -n 1 > "${top}.lvs.report1"

set result [exec cat "${top}.lvs.report1" | awk {{print $1}}]

set fdne_result "runme_lvs.result"
exec touch "$fdne_result"
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "Layout Vs. Schematic (LVS) complete" | tee $fdne_result -a >&@stdout
exec echo "--------------------------------------------------" | tee $fdne_result -a >&@stdout
exec echo "exported layout (GDS)      : $fd_output/${top}.calibre.db" | tee $fdne_result -a >&@stdout
exec echo "exported schematic (SPICE) : $fd_output/${top}.src.net" | tee $fdne_result -a >&@stdout
exec echo "LVS result                 : '$result'" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
