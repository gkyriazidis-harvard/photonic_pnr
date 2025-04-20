#!/bin/tclsh

source "runme_procs.tcl"

set cmd [lindex $argv 0]
set filelist [lindex $argv 1]
set technology_configuration [lindex $argv 2]
set design_configuration [lindex $argv 3]
set mapping_configuration [lindex $argv 4]
set project_configuration [lindex $argv 5]

set fd_output [XML_readExternal "/root/project/variables" \
    "/root/project/digital/outputs/output\[@type='simulation'\]/path" \
$project_configuration]
exec mkdir -p "${fd_output}"

set cmd [string tolower $cmd]
set output "waveforms_${cmd}"
set log "vcs_${cmd}.log"

if { $filelist == "" } {
    set filelist "${fd_output}/filelist_[string tolower $cmd]"
    
    set fdne_tb [XML_readExternals "/root/design/variables" "/root/design/sources/source\[@type='testbench'\]" [list \
        "path" \
        "file" \
    ] "/" $design_configuration]
    if { $cmd == "rtl" } {
        set fdne_rtl [XML_readExternals "/root/design/variables" "/root/design/sources/source\[@type='logic'\]" [list \
            "path" \
            "file" \
        ] "/" $design_configuration]
        set fdne_src $fdne_rtl
    } else {
        set top [XML_readEntry "/root/design/modules/module\[@class='Top'\]" $design_configuration]
        
        set fdne_libs_tech [XML_readExternals "/root/pdk/variables" "/root/pdk/libraries/library\[@type='behavioral'\]/" [list \
            "path" \
            "file" \
        ] "/" $technology_configuration]
        set fdne_libs_map [XML_readExternals "/root/mapping/variables" "/root/mapping/libraries/library\[@type='behavioral'\]/" [list \
            "path" \
            "file" \
        ] "/" $mapping_configuration]
        set fdne_libs [concat $fdne_libs_tech $fdne_libs_map]
        
        if { $cmd == "syn" } {
            set fd_syn [XML_readExternal "/root/project/variables" \
                "/root/project/digital/outputs/output\[@type='synthesis'\]/path" \
            $project_configuration]
            set fdne_top "$fd_syn/export/${top}.syn.v"
        } elseif { $cmd == "pnr" } {
            set fd_pnr [XML_readExternal "/root/project/variables" \
                "/root/project/digital/outputs/output\[@type='physical'\]/path" \
            $project_configuration]
            set fdne_top "$fd_pnr/export/${top}.pnr.v"
        } else {
            puts "Invalid command..."
            exit 1
        }

        set fdne_src [concat $fdne_libs $fdne_top]
    }

    set fo [open "${filelist}" "w"]
    foreach tmp $fdne_tb  { puts $fo [exec realpath $tmp] }
    foreach tmp $fdne_src { puts $fo [exec realpath $tmp] }
    close $fo
}

# Read include dirs
set f [open $filelist r]
set filepaths [split [string trim [read $f]]]
close $f
set dirs []
foreach filepath $filepaths {
    lappend dirs \"[file dirname $filepath]\"
}
set dirs [lsort -unique $dirs]
set dirstring [join $dirs "+"]

set timing [BOOL_check [XML_readEntry "/root/project/digital/simulation/timing" $project_configuration]]
if { $cmd == "rtl" } {
    set timing "0"
}

#set filelist [exec realpath --relative-to="${fd_output}" "${filelist}"]
set filelist [exec realpath "${filelist}"]
cd "${fd_output}"
if { $timing } {
    exec vcs -full64 -sverilog \
        -l "${log}" \
        -f "${filelist}" \
        -o "${output}.vcs" \
        -vcd "${output}.vcd" \
        -Mupdate -R \
        -notice \
        +incdir+$dirstring \
        +lint=TFIPC-L \
        +define+VCS +define+${cmd} \
    >&@stdout
} else {
    exec vcs -full64 -sverilog \
        -l "${log}" \
        -f "${filelist}" \
        -o "${output}.vcs" \
        -vcd "${output}.vcd" \
        -Mupdate -R \
        -notice \
        +incdir+$dirstring \
        +notimingchecks \
        +lint=TFIPC-L \
        +define+VCS +define+${cmd} \
    >&@stdout
}

exec vcd2fsdb "${output}.vcd" -o "${output}.vcd.fsdb" >&@stdout
