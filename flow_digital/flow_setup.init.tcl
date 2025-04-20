# ********************************************************************************
# File             : flow_setup.init.tcl
# Version          : 1.1.0, Tue Aug 13 2024
# Description      : Initializes environment, source and debug procedures
#
# Flow Step        : General Flow
#
# Changelog
#     - v1.0.0: Initial release
#     - v1.1.0: Added redirection support for csource
# ********************************************************************************


# Sources file using relative to script path
#
# Arguments: string       filename
#            list<string> args     ; Redirections [Optional]
# Return:    null
proc csource {filename args} {
    tailcall source [file join [file dirname [pwd]] $filename] {*}$args
}


csource flow_setup.log.tcl

#csource flow_setup.files.tcl
