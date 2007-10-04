#-----------------------------------------------------------------------
# Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# File:         orbix_notify_service.tcl
#
# Description:	An "itadmin" script that can be used to create a
#		Notification Service on the local host.
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
    -register   Register the Notifcation Service on the local machine
    -unregister Unregister the Notifcation Service on the local machine
    -start      Start the Notification Service
    -stop       Stop the Notification Service
    -set_vars   Set or update the server's configuration variables
    -launch_cmd Print the server's launch command
    -exe <name> Use <name> as the executable when printing a usage description 
"
}





#-----------------------------------------------------------------------
# proc process_cmd_line_args
#-----------------------------------------------------------------------

proc process_cmd_line_args {_data} {
	global argc argv

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
		{ {-register}			0		action }
		{ {-unregister}			0		action }
		{ {-start}			0		action }
		{ {-stop}			0		action }
		{ {-set_vars}			0		action }
		{ {-launch_cmd}			0		action }
		{ {-exe}			1		-exe }
		{ {.*\.des}			0		des_script }
	}

	#--------
	# Default values
	#--------
	set data(orig_argv)		$argv
	set data(action)		""
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

	#--------
	# Check that an action (-create, -register, -unregister etc) was
	# specified
	#--------
	if {$data(action) == ""} {
		set msg ""; append msg \
			"You must specify one of: -create, -register, " \
			"-unregister, -start, -stop, " \
			"-set_vars, -launch_cmd"
		puts stderr [box_msg $msg]
		usage $exe_name
		exit 1
	}
}





#-----------------------------------------------------------------------
# proc process_create_action
#-----------------------------------------------------------------------

proc process_create_action {} {
	set sample_file "C:/corbautil/bin/orbix_notify_service_template.des"
	file copy -force $sample_file $::data(des_script)
}





#-----------------------------------------------------------------------
# proc process_unregister_action
#-----------------------------------------------------------------------

proc process_unregister_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		puts stderr $err
		exit 1
	}

	source $data(des_script)
	set var_dir [lindex [split [variable show "o2k.data.root"] {"}] 1]
	set ior_file [file join $var_dir "$env(IT_DOMAIN_NAME)/$orb_name.ior"]
	do_command data file delete $ior_file
	do_command data named_key remove $named_key
}





#-----------------------------------------------------------------------
# proc process_register_action
#-----------------------------------------------------------------------

proc process_register_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		exit 1
	}

	source $data(des_script)
	set var_dir [lindex [split [variable show "o2k.data.root"] {"}] 1]
	set ior_file [file join $var_dir "$env(IT_DOMAIN_NAME)/$orb_name.ior"]
	if {[catch {
		do_command data exec itnotify prepare \
				-ORBname $orb_name \
				-publish_to_file=$ior_file
	} err]} {
		puts stderr $err
		exit 1
	}
	if {$data(want_execute)} {
		set fd [open $ior_file "r"]
		set contents [read $fd]
		close $fd
		set ior [string trim [lindex [split $contents =] end]]
	} else {
		set ior "IOR:..."
	}
	do_command data named_key create -key $named_key $ior
}; # proc process_register_action





#-----------------------------------------------------------------------
# proc process_start_action
#-----------------------------------------------------------------------

proc process_start_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		puts stderr $err
		exit 1
	}
}





#-----------------------------------------------------------------------
# proc process_stop_action
#-----------------------------------------------------------------------

proc process_stop_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		puts stderr $err
		exit 1
	}
}





#-----------------------------------------------------------------------
# proc process_set_vars_action
#-----------------------------------------------------------------------

proc process_set_vars_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		puts stderr $err
		exit 1
	}
}; # proc process_set_vars_action





#-----------------------------------------------------------------------
# proc process_launch_cmd_action
#-----------------------------------------------------------------------

proc process_launch_cmd_action {} {
	global data env

	set cmd [concat {exec orbix_srv_admin} $data(orig_argv)]
	if {[catch {
		eval $cmd >@stdout 2>@stderr
	} err]} {
		puts stderr $err
		exit 1
	}
}; # proc process_launch_cmd_action





#-----------------------------------------------------------------------
# Mainline of the script.
#-----------------------------------------------------------------------

process_cmd_line_args data
#--------
# Invoke any operation on the locator daemon to make sure that it is
# running before we try doing useful work
#--------
if {[catch {
	set dummy [node_daemon list]
   } err]} {
	puts stderr [box_msg $err]
	exit
}

set action $data(action)
process_${action}_action
