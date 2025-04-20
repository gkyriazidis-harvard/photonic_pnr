# ********************************************************************************
# File             : procs_xml.tcl
# Version          : 1.0.1, Sat Jun 1 2024
# Description      : Utility procedures for xml parsing
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.0.1: Added support for first empty entry in XML_readMultiples
# ********************************************************************************


# Verify the sourcing is not affected by the execution path
source [file join [file dirname [info script]] "procs_misc.tcl"]


# Checks format of xml file
#
# Arguments: string filename
# Return:    boolean
proc XML_checkFormat {filename} {
    return [expr {[catch { exec xmllint "$filename" } msg] == 0}]
}


# Reads single attribute from xml file using external/provided variables
#
# Arguments: dict   variables
#            string expression
#            string filename
# Return:    string
proc XML_readExternal {variables expression filename} {
    variable VARS
    set VARS [XML_readVariables "${variables}/descendant::*/" \
        "name" \
        "value" \
        $filename]

    set result [XML_readConfig $expression $filename]
    if { [llength $result] == 1 } {
        set result [lindex $result 0]
    }
    return $result
}

# Reads multiple attributes from xml file using external/provided variables
#
# Arguments: dict   variables
#            string common;            Common base path of expressions
#            list<string> expressions; List of relative, to common, expressions
#            string seperator;         Seperator of values to be returned
#            string filename
# Return:    list<string>
proc XML_readExternals {variables common expressions seperator filename} {
    variable VARS
    set VARS [XML_readVariables "${variables}/descendant::*/" \
        "name" \
        "value" \
        $filename]
    return [XML_readConfigs $common $expressions $seperator $filename]
}


# [Private] Processes single attribute in xml buffer
#
# Arguments: string expression
#            string buffer
# Return:    string
proc XML_processSingle {expression buffer} {
    #set result [exec xmllint --xpath $expression "$filename"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | grep -vE "^(/ > ?)?( +-+)?$"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | grep -vE "(^(/ > ))|(( +-+)$)"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | xargs -0 printf "%s\n " | grep -vE "(^(/ > ))|(( +-+)$)"]
    if {[catch {
        #set result [exec echo "$buffer" | xmllint --shell <(cat) <<<"cat $expression" | grep -vE "(^(/ > ))|(( +-+)$)"]
        #set result [exec echo "$buffer" | xmllint --shell <(cat) <<<"cat $expression"]
        #set result [exec echo $buffer | xmllint --format -]
        set result [exec bash -c "echo '$buffer' | xmllint --shell <(cat) <<<'cat $expression' | grep -vE '(^(/ > ))|(( +-+)$)'" ]
    }]} {
        set result ""
    }
    set output [split $result "\n"]

    set path [split $expression "/"]
    set tag [lindex $path end]
    if {[string index $tag 0] == "\@"} {
        set flag 1
        set tag [string range $tag 1 end]
    } else {
        set tag [regsub {(\[.+\])} $tag ""]
        set flag 0
    }
    for {set i 0} {$i < [llength $output]} {incr i} {
        set text [string trim [lindex $output $i]]
        if { $flag } {
            set groups [regexp -inline "$tag=\"(\\S+)\"" "$text"]
            if { [llength $groups] > 1 } {
                set output [lreplace $output $i $i [lindex $groups 1]]
            } elseif { "$text" == "$tag=\"\"" } {
                set output [lreplace $output $i $i ""]
            } else {
                set output [lreplace $output $i $i "?"]
            }
        } else {
            #set groups [regexp -inline "<${tag}(?:\\s\\S+=\\S+)?>(\\S+)</$tag>" "$text"]
            set groups [regexp -inline "<${tag}(?:.+)?>(\\S+)</$tag>" "$text"]
            if { [llength $groups] > 1 } {
                set output [lreplace $output $i $i [lindex $groups 1]]
                continue
            }
            
            #set groups [regexp -inline "<${tag}(?:\\s\\S+=\\S+)?/>" "$text"]
            set groups [regexp -inline "<${tag}(?:.+)?/>" "$text"]
            if { [llength $groups] == 1 } {
                set output [lreplace $output $i $i ""]
            } else {
                set output [lreplace $output $i $i "?"]
            }
        }
    }
    
    if {[llength $output] == 1} {
        set output [lindex $output 0]
    }
    
    return $output
}

# [Private] Reads single attribute from xml file
#
# Arguments: string expression
#            string filename
# Return:    string
proc XML_readSingle {expression filename} {
    #set result [exec xmllint --xpath $expression "$filename"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | grep -vE "^(/ > ?)?( +-+)?$"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | grep -vE "(^(/ > ))|(( +-+)$)"]
    #set result [exec echo "cat $expression" | xmllint --shell "$filename" | xargs -0 printf "%s\n " | grep -vE "(^(/ > ))|(( +-+)$)"]
    if {[catch {
        set result [exec echo "cat $expression" | xmllint --shell "$filename" | grep -vE "(^(/ > ))|(( +-+)$)"]
    }]} {
        set result ""
    }
    set output [split $result "\n"]
    
    set path [split $expression "/"]
    set tag [lindex $path end]
    if {[string index $tag 0] == "\@"} {
        set flag 1
        set tag [string range $tag 1 end]
    } else {
        set tag [regsub {(\[.+\])} $tag ""]
        set flag 0
    }
    for {set i 0} {$i < [llength $output]} {incr i} {
        set text [string trim [lindex $output $i]]
        if { $flag } {
            set groups [regexp -inline "$tag=\"(\\S+)\"" "$text"]
            if { [llength $groups] > 1 } {
                set output [lreplace $output $i $i [lindex $groups 1]]
            } elseif { "$text" == "$tag=\"\"" } {
                set output [lreplace $output $i $i ""]
            } else {
                set output [lreplace $output $i $i "?"]
            }
        } else {
            #set groups [regexp -inline "<${tag}(?:\\s\\S+=\\S+)?>(\\S+)</$tag>" "$text"]
            set groups [regexp -inline "<${tag}(?:.+)?>(\\S+)</$tag>" "$text"]
            if { [llength $groups] > 1 } {
                set output [lreplace $output $i $i [lindex $groups 1]]
                continue
            }
            
            #set groups [regexp -inline "<${tag}(?:\\s\\S+=\\S+)?/>" "$text"]
            set groups [regexp -inline "<${tag}(?:.+)?/>" "$text"]
            if { [llength $groups] == 1 } {
                set output [lreplace $output $i $i "{}"]
            } else {
                set output [lreplace $output $i $i "?"]
            }
        }
    }
    
    if { [llength $output] == 1} {
        set output [lindex $output 0]
    }
    
    return $output
}

# [Private] Reads multiple attributes from xml file
#
# Arguments: proc   procedure;         Processing procedure of entries
#            string common;            Common base path of expressions
#            list<string> expressions; List of relative, to common, expressions
#            string seperator;         Seperator of values to be returned
#            string filename
# Return:    list<string>
proc XML_readMultiples {procedure common expressions seperator filename} {
    if {[string length $common] > 0} {
        if {[string range $common end end] == "/"} {
            set common [string range $common 0 end-1]
        }
    }
    
    if {[catch {
        #set buffer [exec echo "cat $common" | xmllint --shell "$filename" | grep -vE "(^(/ > ))|(( +-+)$)"]
        set buffer [exec echo "cat $common" | xmllint --shell "$filename" | grep -vE "(^(/ > ))"]
    }]} {
        set buffer ""
    }
    set delimeter " -------"
    set splitchar "\u0080"; #any restriced character
    set entries [split [string map [list $delimeter $splitchar] $buffer] $splitchar]

    set path [split $common "/"]
    set tag [lindex $path end]    
    if {[string index $tag 0] == "\@"} {
        set tag [string range $tag 1 end]
    } else {
        set tag [regsub {(\[.+\])} $tag ""]
    }

    set result [list]
    if { [PROC_exists $seperator] } {
        for {set i 0} {$i < [llength $entries]} {incr i} {
            set buffer [list]
            set entry [lindex $entries $i]
            foreach {expression} $expressions {
                lappend buffer [$procedure "$tag/$expression" $entry]
            }
            lappend result [$seperator {*}$buffer]
        }
    } else {
        for {set i 0} {$i < [llength $entries]} {incr i} {
            set temp [$procedure "$tag/[lindex $expressions 0]" [lindex $entries $i]]
            if { $seperator == " " } {
                lappend result "{$temp}"
            } else {
                lappend result "$temp"
            }
        }
        
        foreach {expression} [lrange $expressions 1 end] {
            for {set i 0} {$i < [llength $entries]} {incr i} {
                set temp [$procedure "$tag/$expression" [lindex $entries $i]]
                set prev [lindex $result $i]
                if { $seperator == " " } {
                    append prev "$seperator" "{$temp}"
                } else {
                    append prev "$seperator" "$temp"
                }
                set result [lreplace $result $i $i $prev]
            }
        }
        
        set size [expr "[string length $seperator]*([llength $expressions]-1)"]
        for {set i 0} {$i < [llength $result]} {incr i} {
            if { [string length [lindex $result $i]] == $size } {
                set result [lreplace $result $i $i]
            }
        }
    }
    
    return $result
}


# Reads single entry from xml file;
# Returns null if attribute not found
#
# Arguments: string expression
#            string filename
# Return:    string
proc XML_readEntry {expression filename} {
    set tmp [XML_readSingle $expression $filename]
    if { [llength $tmp] == 1 } {
        if { [string length [lindex $tmp 0]] == 0 } {
            return
        }
    }
    return $tmp
}

# Reads multiple entries from xml file
#
# Arguments: string common;            Common base path of expressions
#            list<string> expressions; List of relative, to common, expressions
#            string seperator;         Seperator of values to be returned
#            string filename
# Return:    list<string>
proc XML_readEntries {common expressions seperator filename} {
    return [XML_readMultiples XML_processSingle $common $expressions $seperator $filename]
}


# [Private] Processes single attribute in xml buffer using variables
#
# Arguments: [global] dict VARS
#            string expression
#            string buffer
# Return:    string
proc XML_processConfig {expression buffer} {
    global VARS
    set result [XML_processSingle $expression $buffer]
    if {[info exists VARS]} {
        set result [VARS_apply $result $VARS]
    }
    return $result
}

# Reads single configuration from xml file using variables
#
# Arguments: [global] dict VARS
#            string expression
#            string filename
# Return:    string
proc XML_readConfig {expression filename} {
    global VARS
    set result [XML_readSingle $expression $filename]
    if {[info exists VARS]} {
        set result [VARS_apply $result $VARS]
    }
    return $result
}

# Reads multiple configurations from xml file using variables
#
# Arguments: [global] dict VARS
#            string common;            Common base path of expressions
#            list<string> expressions; List of relative, to common, expressions
#            string seperator;         Seperator of values to be returned
#            string filename
# Return:    list<string>
proc XML_readConfigs {common expressions seperator filename} {
    return [XML_readMultiples XML_processConfig $common $expressions $seperator $filename]
}


# Reads variables from xml file
#
# Arguments: string common; Base path of variables
#            string key;    Name of key element
#            string value;  Name of value element
#            string filename
# Return:    dict
proc XML_readVariables {common key value filename} {
    if {[string length $common] > 0} {
        if {[string range $common end end] != "/"} {
            set common "$common/"
        }
    }
    
    #set names [XML_readSingle $common$key $filename]
    #set values [XML_readSingle $common$value $filename]

    #set result []
    #for {set i 0} {$i < [llength $names]} {incr i} {
    #    dict set result [lindex $names $i] [lindex $values $i]
    #}
    
    set result []
    set buffer [XML_readEntries $common [list \
        "$key" \
        "$value" \
    ] " " $filename]
    foreach {tmp} $buffer {
        dict set result [lindex $tmp 0] [lindex $tmp 1]
    }

    return $result
}
