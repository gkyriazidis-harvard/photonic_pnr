#!/bin/tclsh

source "runme_procs.tcl"

set technology_configuration [lindex $argv 0]
set design_configuration     [lindex $argv 1]
set mapping_configuration    [lindex $argv 2]
set project_configuration    [lindex $argv 3]

set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
set tech_libraries [XML_readEntry "/root/pdk/libraries/names/name" $technology_configuration]
set map_libraries [XML_readEntry "/root/mapping/libraries/names/name" $mapping_configuration]
set library_name [XML_readEntry "/root/project/digital/library/name" $project_configuration]
set fd_physical [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='physical'\]/path" \
$project_configuration]
set fd_virtuoso [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='virtuoso'\]/path" \
$project_configuration]
set fd_output [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='library'\]/path" \
$project_configuration]

# ==================================================
# remove existing library and log file
# --------------------------------------------------
exec mkdir -p "${fd_output}"
exec rm -rf "${fd_output}/${library_name}"

# give full path in order to create logFile in this directory,
# otherwise the log file will be made in runDir
set logFile "[pwd]/${fd_virtuoso}/strmin.log"
# remove logFile from previous run
exec rm -rf $logFile

# create custom .refLibs file
set refLibs [list {*}$tech_libraries {*}$map_libraries]
puts $refLibs
exec printf "%s\n" {*}$refLibs > strmin.refLibs
set fdne_refLibList "strmin.refLibs"

set fdne_layerMap [XML_readExternals "/root/pdk/variables" "/root/pdk/maps/map\[@type='layers-virtuoso'\]/" [list \
    "path" \
    "file" \
] "/" $technology_configuration]
set tech [XML_readEntry "/root/pdk/details/name" $technology_configuration]

set scale [XML_readEntry "/root/pdk/physical/floorplan/scale" $technology_configuration]
if {[STRING_empty $scale]} {
    set scale 1
}

# ==================================================
# stream in
# --------------------------------------------------
# note that, using -attachTechFileOfLib creates the library as a design library (instead of a technology library)
# using -techRefs (e.g., -techRefs 'n1') will create the library as a technology library,
# using neither -attachTechFileOfLib nor -techRefs will create a technology library as well
#set fd_output [exec realpath --relative-to="${fd_virtuoso}" "${fd_output}"]
set fd_output [exec realpath "${fd_output}"]
#set fd_physical [exec realpath --relative-to="${fd_virtuoso}" "${fd_physical}"]
set fd_physical [exec realpath "${fd_physical}"]
#set fdne_refLibList [exec realpath --relative-to="${fd_virtuoso}" "${fdne_refLibList}"]
set fdne_refLibList [exec realpath "${fdne_refLibList}"]
cd "${fd_virtuoso}"
exec strmin \
    -strmFile            [exec readlink -f ${fd_physical}/export/${top}.gds] \
    -runDir              ${fd_output} \
    -library             ${library_name} \
    -layerMap            ${fdne_layerMap} \
    -scale               ${scale} \
    -scaleTextHeight     ${scale} \
    -logFile             ${logFile} \
    -attachTechFileOfLib ${tech} \
    -view                "layout" \
    -refLibList          ${fdne_refLibList} \
    -noDetectVias \
    -replaceBusBitChar \
>&@stdout
# use -noDetectVias to avoid streaming in large vias for a7
