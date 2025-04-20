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
    "/root/project/digital/outputs/output\[@type='drc'\]/path" \
    $project_configuration]
set fdne_rul_template [XML_readExternals "/root/pdk/variables" "/root/pdk/rules/rule\[@type='drc'\]/" [list \
    "path" \
    "file" \
] "/" $technology_configuration]

if { [file exists "$fd_output"] } {
    exec rm -rf "$fd_output"
}
exec mkdir -p "$fd_output"

# create drc deck from template
exec sed -e "s/\${top}/${top}/g" "$fdne_rul_template" > "${fd_output}/_drcRules.drc.${top}.drc_"

# remove existing gds
if { [file exists "${fd_output}/${top}.calibre.db"] } {
    echo "removing existing gds!!!"
    exec rm "${fd_output}/${top}.calibre.db"
}

set fdne_layerMap [XML_readExternals "/root/pdk/variables" "/root/pdk/maps/map\[@type='layers-virtuoso'\]/" [list \
    "path" \
    "file" \
] "/" $technology_configuration]

#set fd_output [exec realpath --relative-to="${fd_virtuoso}" "${fd_output}"]
set fd_output [exec realpath "${fd_output}"]
cd "${fd_virtuoso}"
exec strmout \
    -library     "${library_name}" \
    -strmFile    "${top}.calibre.db" \
    -topCell     "${top}" \
    -view        "layout" \
    -runDir      "${fd_output}" \
    -logFile     "PIPO.LOG.${top}" \
    -summaryFile "PIPO.SUM.${top}" \
    -layerMap    "${fdne_layerMap}" \
    -case        "Preserve" \
    -convertDot  "node" \
>&@stdout

# calibre -drc dumps a bunch of .db files, so cd to $fd_output first
cd "$fd_output"

set fdne_rul [exec readlink -f _drcRules.drc.${top}.drc_]
exec calibre \
    -drc \
    -hier \
    -nowait \
    -turbo -turbo_all \
    $fdne_rul \
    | tee "runme_drc.calibre.tee.log" \
>&@stdout

exec grep "TOTAL DRC Results Generated" "${top}.drc.summary" > "${top}.drc.summary1"

set result [exec cat "${top}.drc.summary1" | sed {s/TOTAL DRC Results Generated:\s*\([0-9]*\)\s*.*/\1/g}]

set fdne_result "runme_drc.result"
exec touch "$fdne_result"
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "Design Rule Check (DRC) complete" | tee $fdne_result -a >&@stdout
exec echo "--------------------------------------------------" | tee $fdne_result -a >&@stdout
exec echo "exported layout (GDS)    : $fd_output/${top}.calibre.db" | tee $fdne_result -a >&@stdout
exec echo "number of DRC violations : '$result'" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
exec echo "==================================================" | tee $fdne_result -a >&@stdout
