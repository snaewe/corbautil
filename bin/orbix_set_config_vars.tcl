#-----------------------------------------------------------------------
# Copyright (c) 2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# File:         orbix_set_config_vars.tcl
#
# Description:	An "itadmin" script that can be used to set configuration
#		variables for an Orbix application.
#
# Author:	Ciaran.McHale@iona.com, Principal Consultant
#-----------------------------------------------------------------------


source "C:/corbautil/bin/orbix_admin_lib.tcl"





#-----------------------------------------------------------------------
# proc usage
#
# Description:  Print a usage statement.
#-----------------------------------------------------------------------

proc usage {{exe_name ""}} {
	global argv0

	if {$exe_name == ""} { set exe_name "itadmin $argv0" }
	puts stderr "
usage: $exe_name \[options\] file.des

options are:
    -s          Silent mode
    -n          Do not execute commands. Just show them
    -h          Print this usage statement
    -create     Create a starting-point description file (Tcl script)
    -exe <name> Use <name> as the executable when printing a usage description 
"
}





#-----------------------------------------------------------------------
# proc process_cmd_line_args
#-----------------------------------------------------------------------

proc process_cmd_line_args {_data} {
	global argc

	upvar $_data	data

	#----
	# Specify allowable command-line arguments. The format
	# is a list of tuples where each tuple is:
	#
	#	{ regular expression		extra-arg?	name }
	#	  ------------------		----------	----
	#----
	set arg_info_list {
		{ {-s}				0		-s }
		{ {-n}				0		-n }
		{ {-h}				0		-h }
		{ {-create}			0		action }
		{ {-exe}			1		-exe }
		{ {.*\.des}			0		des_script }
	}

	#--------
	# Default values
	#--------
	set data(action)		"set_vars"
	set data(want_diagnostics)	"1"
	set data(want_execute)		"1"
	set data(des_script)		""
	set exe_name			""

	#--------
	# Process the command-line arguments until there are none left.
	#--------
	while {$argc > 0} {
		idlgen_getarg $arg_info_list arg param name
		switch -exact -- $name {
		    -s		{
				  set data(want_diagnostics) 0
				  set data(want_execute) 1
				}
		    -n		{
				  set data(want_diagnostics) 1
				  set data(want_execute) 0
				}
		    -h		{ usage $exe_name; exit 0 }
		    -exe	{ set exe_name $param }
		    action	{ set data(action) [string range $arg 1 end]}
		    des_script	{ set data(des_script) $arg }
		    default	{
				  set msg "Unknown option '$arg'"
				  puts stderr [box_msg $msg]
				  usage $exe_name
				  exit 1
		    		}
		}; # switch
	}; # while

	#--------
	# Check that a description file was specified
	#--------
	if {$data(des_script) == ""} {
		set msg "You must specify a description file"
		puts stderr [box_msg $msg]
		usage $exe_name
		exit 1
	}
}





#-----------------------------------------------------------------------
# proc process_create_action
#-----------------------------------------------------------------------

proc process_create_action {} {
	global data tcl_platform

	set app_name [file root [file tail $data(des_script)]]
	set user $tcl_platform(user)
	puts "itadmin: creating $data(des_script)"
	if {[catch {
		set fd [open $data(des_script) "w"]
		puts -nonewline $fd \
"#-----------------------------------------------------------------------
# This file is a simple Tcl script that contains assignment statements
# (of the form \"set name value\"). The values in this file are then
# used by the \"orbix_set_config_vars.tcl\" script to execute various
# \"itadmin\" commands in order to set runtime configuration variables
# for an Orbix application.
#-----------------------------------------------------------------------

#--------
# The orb_name specifies the configuration scope where configuration
# variables will be set.
#--------
set orb_name          \"acme.uk.$app_name\"

#--------
# Each line in runtime_config_variables is a triplet of the form:
#	type name value
# The type can be one of: long, bool, list, string or double
# list values are comma-separated strings
# bool values can be: true or false
#--------
set runtime_config_variables {
    string  plugins:local_log_stream:filename	\"server.log\"
    list    event_log:filters			\"*=WARN+ERR+FATAL\"
    long    thread_pool:high_water_mark		\"10\"
    long    thread_pool:low_water_mark		\"10\"
    long    thread_pool:initial_threads		\"10\"
    long    thread_pool:max_queue_size		\"500\"
}
"
		close $fd
	} err]} {
		puts stderr [box_msg "Error creating $data(des_script): $err"]
		exit 1
	}
}; # proc process_create_action





#-----------------------------------------------------------------------
# proc read_config_script_and_do_sanity_checks
#-----------------------------------------------------------------------

proc read_config_script_and_do_sanity_checks {} {
	global data env errorInfo tcl_platform

	#--------
	# Read in the config script
	#--------
	if {[catch {
		source $data(des_script)
	} err]} {
		puts stderr [box_msg "Error in $data(des_script)"]
		puts stderr "\n$errorInfo\n"
		exit 1
	}


	#--------
	# Sanity check
	# Check that all the necessary config values have been set
	# and copy them into the data() array for easier access.
	#--------
	set des_var_name_list {
		orb_name			orb_name
		runtime_config_variables	runtime_cfg_vars
	}
	foreach {var_name data_name} $des_var_name_list {
		if {![info exists $var_name]} {
			set msg ""; append msg \
				"$data(des_script): you must set a value " \
				"for '$var_name'"
			puts stderr [box_msg $msg]
			exit 1
		}
		set data($data_name) [set $var_name]
	}

	#--------
	# Sanity check on the runtime config variables
	#--------
	set len [llength $data(runtime_cfg_vars)]
	if {$len % 3 != 0} {
		set msg ""; append msg \
			"$data(des_script): 'runtime_config_variables' must " \
			"contain triplets of items"
		puts stderr [box_msg $msg]
		exit 1
	}
	for {set i 2} {$i < $len} {incr i 3} {
		set str [lindex $data(runtime_cfg_vars) $i]
		escape_special_chars str
		set data(runtime_cfg_vars) \
			[lreplace $data(runtime_cfg_vars) $i $i $str]
	}
}; # proc read_config_script_and_do_sanity_checks





#-----------------------------------------------------------------------
# proc process_set_vars_action
#-----------------------------------------------------------------------

proc process_set_vars_action {} {
	global data

	#--------
	# Create a scope hierarchy that parallels the orb name
	#--------
	foreach orb_scope [list_orb_hierarchy $data(orb_name)] {
		if {![does_scope_exist $orb_scope]} {
			do_command data scope create $orb_scope
		}
	}

	#--------
	# Set the configuration variables in the specified scope
	#--------
	set_vars_in_one_scope $data(runtime_cfg_vars) $data(orb_name)
}; # proc process_set_vars_action





#-----------------------------------------------------------------------
# Mainline of the script.
#-----------------------------------------------------------------------

process_cmd_line_args data
#--------
# Invoke any operation on the configuration domain to make sure that
# it is accessible before we try doing useful work
#--------
if {[catch {
	set dummy [scope list]
   } err]} {
	puts stderr [box_msg $err]
	exit
}

set action $data(action)
if {$action != "create"} {
	read_config_script_and_do_sanity_checks
}
process_${action}_action
