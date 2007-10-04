#-----------------------------------------------------------------------
# Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# File:         orbix_ns_on_fixed_port.tcl
#
# Description:	An "itadmin" script that can be used to modify an
#		existing Orbix configuration in order to get the Naming
#		Service listening on a fixed port. This script automates
#		most of the steps discussed in the following Knowledge
#		Base article:
#
#			http://www.iona.com/support/articles/3757.360.xml
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
usage: $exe_name \[options\]

options are:
    -s           Silent mode
    -n           Do not execute commands. Just show them
    -l <host>    Local host name           (example: foo)
    -f <host>    Fully-qualified host name (example: foo.bar.com)
    -port <port> Fixed port
    -h           Print this usage statement
    -exe <name>  Use <name> as the executable when printing a usage description 
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
		{ {-l}				1		-l }
		{ {-f}				1		-f }
		{ {-port}			1		-port }
		{ {-exe}			1		-exe }
	}

	#--------
	# Default values
	#--------
	set data(orig_argv)		$argv
	set data(action)		""
	set data(want_diagnostics)	"1"
	set data(want_execute)		"1"
	set data(l_host)		""
	set data(f_host)		""
	set data(port)			""
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
		    -l		{ set data(l_host) $param }
		    -f		{ set data(f_host) $param }
		    -port	{ set data(port) $param }
		    -exe	{ set exe_name $param }
		    default	{
				  set msg "Unknown option '$arg'"
				  puts stderr [box_msg $msg]
				  usage $exe_name
				  exit 1
		    		}
		}; # switch
	}; # while

	#--------
	# Check that -port <number> was specified
	#--------
	if {$data(port) == ""} {
		set msg "You must specify -port <number>"
		puts stderr [box_msg $msg]
		usage $exe_name
		exit 1
	}
	#--------
	# Check that -l <host> was specified
	#--------
	if {$data(l_host) == ""} {
		set msg "You must specify -l <host>"
		puts stderr [box_msg $msg]
		usage $exe_name
		exit 1
	}
	#--------
	# Check that -f <host> was specified
	#--------
	if {$data(f_host) == ""} {
		set msg "You must specify -f <fully-qualified-host>"
		puts stderr [box_msg $msg]
		usage $exe_name
		exit 1
	}

}





#-----------------------------------------------------------------------
# proc process_register_action
#-----------------------------------------------------------------------

proc do_work {} {
	global data env
	set l_host $data(l_host)
	set f_host $data(f_host)
	set port $data(port)
	set domain $env(IT_DOMAIN_NAME)

	#--------
	# Make changes to some NS-related configuration variables
	#--------
	do_command data variable modify \
				-scope iona_services.naming \
				-type list \
				-value "+${f_host}:$port" \
				plugins:naming_cluster:iiop:addr_list

	do_command data variable modify \
				-scope iona_services.naming.$l_host \
				-type list \
				-value "${f_host}:$port" \
				plugins:naming:iiop:addr_list

	do_command data variable modify \
				-scope iona_services.naming.$l_host \
				-type bool \
				-value "true" \
				plugins:naming:direct_persistence

	#--------
	# Now kill the NS and then run it in prepare mode, and save its
	# IORs to a file
	#--------
	set var_dir [lindex [split [variable show "o2k.data.root"] {"}] 1]
	set ior_file [file join $var_dir \
			"$domain/iona_services.naming.$l_host.ior"]
	do_command data ns stop
	do_command data after 2000
	if {[catch {
		do_command data exec itnaming prepare \
				-ORBname iona_services.naming.$l_host \
				-publish_to_file=$ior_file
	} err]} {
		puts stderr $err
		exit 1
	}

	#--------
	# Extract the two IORs (ns_ior and single_ns_ior) from the file.
	#--------
	if {$data(want_execute)} {
		set fd [open $ior_file "r"]
		set contents [read $fd]
		close $fd
		set lines [split $contents "\n"]
		set line1 [lindex $lines 0]
		set line2 [lindex $lines 1]
		set ns_ior [string trim [lindex [split $line1 =] end]]
		set single_ns_ior [string trim [lindex [split $line2 =] end]]
	} else {
		set ns_ior "IOR:..."
		set single_ns_ior "IOR:..."
	}

	#--------
	# Copy the IORs back into some more configuration variables
	#--------
	do_command data variable modify \
			-type string \
			-value $ns_ior \
			initial_references:NameService:reference

	do_command data variable modify \
			-type list \
			-value "iona_services.naming.$l_host=$single_ns_ior" \
			IT_NameServiceReplicas

	do_command data variable modify \
			-type string \
			-value $single_ns_ior \
			initial_references:IT_MasterNameService:reference

	#--------
	# Register the Naming Service with the Named Key.
	#--------
	do_command data named_key remove NameService
	do_command data named_key create -key NameService $ns_ior

	puts "\n"
	puts "To finish, you have to do the following..."
	puts "\t1. Ensure that \"start_${domain}_services\" starts itnaming"
	puts "\t2. Run \"stop_${domain}_services\""
	puts "\t   (ignore any error messages regarding the Naming Service)"
	puts "\t3. Run \"start_${domain}_services\""
}; # proc process_register_action





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

#--------
# Do the real work
#--------
do_work
