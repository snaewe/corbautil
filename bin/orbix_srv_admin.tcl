#-----------------------------------------------------------------------
# Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# File:         orbix_srv_admin.tcl
#
# Description:	An "itadmin" script that can be used to perform common
#		administration tasks on an Orbix server, based on
#		information specified in a Tcl configuration script.
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
    -register   Register the application's details
    -unregister Unregister the application's details
    -start      Start the server
    -stop       Stop the server
    -set_vars   Set or update the server's configuration variables
    -launch_cmd Print the server's launch command
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
	global data tcl_platform

	set srv_name [file root [file tail $data(des_script)]]
	set user $tcl_platform(user)
	puts "itadmin: creating $data(des_script)"
	set host_list [node_daemon list]
	if {[catch {
		set fd [open $data(des_script) "w"]
		puts -nonewline $fd \
"#-----------------------------------------------------------------------
# This file is a simple Tcl script that contains assignment statements
# (of the form \"set name value\"). The values in this file are then
# used by the \"orbix_srv_admin.tcl\" script to execute various \"itadmin\"
# commands in order to perform common adminstration tasks on an Orbix
# server, such as (un)register it, start/stop it, and update its
# runtime configuration variables.
#-----------------------------------------------------------------------

set orb_name          \"acme.uk.$srv_name\"
set process_name      \$orb_name
set root_poa_name     \$orb_name
set description       \"\"
set startup_mode      \"on_demand\"; # on_demand or disable
#--------
# File names on Windows can be expressed as \"C:/full/path/to/file\"
# or as \"C:\\\\full\\\\path\\\\to\\\\file\"
#--------
set executable        \"/full/path/to/executable\"
set cmd_line_args     \"x y z\"
set working_directory \"/full/path/to/current/working/directory\"
#--------
# If \"env_var_list\" is set to an empty list \"{}\" then the launched
# application inherits all the environment variables from the node
# daemon that launched it. If \"env_var_list\" is not empty then the
# launched application does not inherit any environment variables
# from the node daemon, and instead has only the environment variables
# listed in \"env_var_list\".
#--------
set env_var_list      \[list \\
			\"PATH=\$env(PATH)\" \\
			\"CLASSPATH=\$env(CLASSPATH)\" \\
"
		if {$tcl_platform(platform) == "windows"} {
			puts -nonewline $fd \
"			\"SYSTEMROOT=\$env(SYSTEMROOT)\" \\
"
		}
		puts $fd \
"			\"IT_CONFIG_DOMAINS_DIR=\$env(IT_CONFIG_DOMAINS_DIR)\" \\
			\"IT_DOMAIN_NAME=\$env(IT_DOMAIN_NAME)\" \\
			\"IT_LICENSE_FILE=\$env(IT_LICENSE_FILE)\" \\
		      \]
#--------
# If \"node_daemon_list\" contains several entries then the server
# will be registered as a replicated server and \"load_balancer\"
# specifies the load-balancing policy (random or round_robin)
# to be used. If \"node_daemon_list\" contains just one entry then
# \"load_balancer\" is ignored.
#--------
set node_daemon_list  \{$host_list\}
set load_balancer     \"random\"; # random or round_robin
#--------
# UNIX-specific. These entries are ignored on Windows
#--------
set group             \"nobody\"
set user              \"$user\"
set umask             \"002\"

#--------
# Each line in poa_hierarchy is a pair of the form:
#       lifespan full/path/to/poa-name
# where lifespan can be one of: transient or persistent
#--------
set poa_hierarchy {
    persistent  FooFactory
    persistent  FooFactory/Foo
    transient   FooFactory/Foo/FooIterator
    persistent  Administration
}


#--------
# Each of the contexts listed will be created in the name service, if
# do not already exist.
#--------

set naming_service_contexts {
    acme/uk/sample
    acme/uk/production
}


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


#--------
# Each line in runtime_config_variables_replica_<number> is
# similar to that in runtime_config_variables. However, the
# variables will be set in the scope for a replica
#
# If the server is not registered as a replicated server
# (that is, with more than one node daemon) then the
# runtime_config_variables_replica_<number> scopes are ignored.
#--------

set runtime_config_variables_replica_1 {
    string  plugins:local_log_stream:filename	\"server.replica_1.log\"
}

set runtime_config_variables_replica_2 {
    string  plugins:local_log_stream:filename	\"server.replica_2.log\"
}

set runtime_config_variables_replica_3 {
    string  plugins:local_log_stream:filename	\"server.replica_3.log\"
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
	# Migration aid. Make "naming_service_contexts" optional.
	#--------
	if {![info exists naming_service_contexts]} {
		set naming_service_contexts {}
	}


	#--------
	# Sanity check
	# Check that all the necessary config values have been set
	# and copy them into the data() array for easier access.
	#--------
	set des_var_name_list {
		process_name			process_name
		orb_name			orb_name
		root_poa_name			root_poa_name
		description			description
		startup_mode			startup_mode
		node_daemon_list		node_daemons
		load_balancer			load_bal
		executable			exe_path
		cmd_line_args			exe_args
		working_directory		directory
		group				group_name
		user				user_name
		umask				umask
		env_var_list			environment
		poa_hierarchy			poa_hierarchy
		runtime_config_variables	runtime_cfg_vars
		naming_service_contexts		naming_service_contexts
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
	set len [llength $data(node_daemons)]
	if {$len > 1} {
		for {set i 1} {$i <= $len} {incr i} {
			if {[info exist runtime_config_variables_replica_$i]} {
				set data(runtime_cfg_vars_replica_$i) \
				     [set runtime_config_variables_replica_$i]
			} else {
				set data(runtime_cfg_vars_replica_$i) {}
			}
		}
	}


	#--------
	# Escape any special characters in environment variables to
	# prevent Tcl's interpretation of them
	#--------
	set tmp_env ""
	set sep ""
	foreach name_var $data(environment) {
		if {![regexp {.+\=.+} $name_var]} {
			set msg ""; append msg \
				"$data(des_script): env_var_list must " \
				"contain entries of form \"NAME=VALUE\" " \
				"(I'm complaining about \"$name_var\")"
			puts stderr [box_msg $msg]
			exit 1
		}
		if {[string first " " $name_var] != -1} {
			regsub {\=.*} $name_var {} name
			set msg ""; append msg \
				"$data(des_script): a limitation of Orbix " \
				"means that you cannot have spaces " \
				"in the values of environment variables " \
				"(I'm complaining about \"$name\")"
			puts stderr [box_msg $msg]
			exit 1
		}
		escape_special_chars name_var
		append tmp_env "$sep$name_var"
		set sep " "
	}
	set data(environment) $tmp_env

	#--------
	# Add -ORB... command-line arguments if they are not already
	# in the command-line and there are no corresponding
	# environment variables
	#--------
	add_ORB_exe_args

	#--------
	# Escape any special characters in other variables to
	# prevent Tcl's interpretation of them
	#--------
	escape_special_chars data(exe_path)
	escape_special_chars data(exe_args)
	escape_special_chars data(directory)
	escape_special_chars data(description)

	#--------
	# Sanity check on the POA hierarchy
	#--------
	if {[llength $data(poa_hierarchy)] % 2 != 0} {
		set msg ""; append msg \
			"$data(des_script): 'poa_hierarchy' must contain " \
			"pairs of items"
		puts stderr [box_msg $msg]
		exit 1
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

	#--------
	# Set plugins:poa:root_name from data(root_poa_name)
	#--------
	lappend data(runtime_cfg_vars) \
			string plugins:poa:root_name $data(root_poa_name)

	#--------
	# Record whether or not replication is being used
	#--------
	if {[llength $data(node_daemons)] < 2} {
		set data(using_replicas) 0
		set data(node_daemons) [lindex $data(node_daemons) 0]
	} else {
		set data(using_replicas) 1
	}
}; # proc read_config_script_and_do_sanity_checks





#-----------------------------------------------------------------------
# proc process_unregister_action
#-----------------------------------------------------------------------

proc process_unregister_action {} {
	global data

	set root_poa_name $data(root_poa_name)
	if {$root_poa_name != ""} {
		do_command data poa remove $root_poa_name
	} else {
		foreach {lifespan poa} $data(poa_hierarchy) {
			if {[string first "/" $poa] == -1} {
				#--------
				# It is a top-level POA, so remove it
				#--------
				do_command data poa remove $poa
			}
		}; # foreach {lifespan poa}
	}
	if {$data(using_replicas)} {
		set num_replicas [llength $data(node_daemons)]
		for {set i 1} {$i <= $num_replicas} {incr i} {
			do_command data orbname remove \
					$data(orb_name).replica_$i
			do_command data process remove \
					$data(process_name).replica_$i
		}
		do_command data scope remove $data(orb_name)
	} else {
		do_command data orbname remove $data(orb_name)
		do_command data scope   remove $data(orb_name)
		do_command data process remove $data(process_name)
	}
}





#-----------------------------------------------------------------------
# proc process_register_action
#-----------------------------------------------------------------------

proc process_register_action {} {
	global data

	set process_name  $data(process_name)
	set orb_name      $data(orb_name)
	set root_poa_name $data(root_poa_name)
	if {$data(startup_mode) == "disable"} {
		set disable_mode 1
	} else {
		set disable_mode 0
	}
	if {$data(exe_args) != ""} { set data(exe_args) " $data(exe_args)" }

	#--------
	# Determine $root_poa_prefix
	#--------
	if {$root_poa_name == ""} {
		set root_poa_prefix ""
	} else {
		set root_poa_prefix "$root_poa_name/"
	}

	#--------
	# Get a list of the existing ORBs and existing scopes
	#--------
	if {[catch {
		set existing_orb_list   [orbname list]
		set existing_scope_list [scope list]
	} err]} {
		puts [box_msg $err]
		exit 1
	}

	#--------
	# Register the process
	#--------
	if {$data(using_replicas)} {
		set i 1
		foreach node_d $data(node_daemons) {
			set args [add_orbname_arg $data(exe_args) \
					"$data(orb_name).replica_$i"]
			if {$disable_mode} {
				do_command data process create \
					-description $data(description) \
					-startupmode "$data(startup_mode)" \
					$process_name.replica_$i
			} else {
				do_command data process create \
					-description $data(description) \
					-startupmode "$data(startup_mode)" \
					-pathname "$data(exe_path)" \
					-args $args \
					-directory "$data(directory)" \
					-umask $data(umask) \
					-group "$data(group_name)" \
					-user "$data(user_name)" \
					-node_daemon "$node_d" \
					$process_name.replica_$i
				if {[llength $data(environment)] != 0} {
					do_command data process modify \
					    -env $data(environment) \
					    $process_name.replica_$i
				}
			}
			incr i
		}
	} elseif {$disable_mode} {
		do_command data process create \
			-description $data(description) \
			-startupmode "$data(startup_mode)" \
			$process_name
	} else {
		set args [add_orbname_arg $data(exe_args) "$data(orb_name)"]
		do_command data process create \
			-description $data(description) \
			-startupmode "$data(startup_mode)" \
			-pathname "$data(exe_path)" \
			-args $args \
			-directory "$data(directory)" \
			-umask $data(umask) \
			-group "$data(group_name)" \
			-user "$data(user_name)" \
			-node_daemon "$data(node_daemons)" \
			$process_name
		if {[llength $data(environment)] != 0} {
			do_command data process modify \
				-env $data(environment) \
				$process_name
		}
	}


	#--------
	# Create a scope hierarchy that parallels the orb name
	#--------
	foreach orb_scope [list_orb_hierarchy $data(orb_name)] {
		if {[lsearch -exact $existing_scope_list $orb_scope] == -1} {
			do_command data scope create $orb_scope
		}
	}

	#--------
	# Create the desired hierarchy in the Naming Service.
	#--------
	foreach ns_path $data(naming_service_contexts) {
		create_ns_context $ns_path
	}


	#--------
	# Create/modify the ORB(s) to associate it/them with the process(es)
	#--------
	if {$data(using_replicas)} {
		set replica_orb_list ""
		set sep ""
		set i  1
		foreach node_d $data(node_daemons) {
			set replica_orb $orb_name.replica_$i 
			set replica_process $process_name.replica_$i 
			append replica_orb_list "$sep$replica_orb"
			set sep ","
			if {[lsearch -exact $existing_scope_list $replica_orb] \
			     == -1} \
			{
				do_command data scope create $replica_orb
			}
			if {[lsearch -exact $existing_orb_list $replica_orb] \
				== -1} \
			{
				set create_or_modify "create"
			} else {
				set create_or_modify "modify"
			}
			do_command data orbname $create_or_modify \
				-process $replica_process $replica_orb
			incr i
		}
	} else {
		if {[lsearch -exact $existing_orb_list $data(orb_name)] == -1} \
		{
			set create_or_modify "create"
		} else {
			set create_or_modify "modify"
		}
		do_command data orbname $create_or_modify \
				-process $process_name $data(orb_name)
	}


	#--------
	# Set/update the runtime configuration variables
	#--------
	process_set_vars_action

	#--------
	# Register each POA with the locator
	#--------
	if {$root_poa_name != ""} {
		do_command data poa create -transient $root_poa_name
	}
	foreach {lifespan poa} $data(poa_hierarchy) {
		if {$lifespan == "transient"} {
			do_command data poa create \
				-transient ${root_poa_prefix}$poa
		} elseif {$data(using_replicas)} {
			do_command data poa create \
					-replicas "$replica_orb_list" \
					-load_balancer $data(load_bal) \
					${root_poa_prefix}$poa
		} else {
			do_command data poa create \
					-orbname $data(orb_name) \
					${root_poa_prefix}$poa
		}
	}; # foreach {lifespan poa}
}; # proc process_register_action





#-----------------------------------------------------------------------
# proc process_start_action
#-----------------------------------------------------------------------

proc process_start_action {} {
	global data

	set process_name $data(process_name)
	if {$data(using_replicas)} {
		set i 1
		foreach node_d $data(node_daemons) {
			do_command data process start $process_name.replica_$i
			incr i
		}
	} else {
		do_command data process start $process_name
	}
}





#-----------------------------------------------------------------------
# proc process_stop_action
#-----------------------------------------------------------------------

proc process_stop_action {} {
	global data

	set process_name $data(process_name)
	if {$data(using_replicas)} {
		set i 1
		foreach node_d $data(node_daemons) {
			do_command data process stop $process_name.replica_$i
			incr i
		}
	} else {
		do_command data process stop $process_name
	}
}





#-----------------------------------------------------------------------
# proc process_set_vars_action
#-----------------------------------------------------------------------

proc process_set_vars_action {} {
	global data

	set_vars_in_one_scope $data(runtime_cfg_vars) $data(orb_name)

	#--------
	# Now do the same thing for each replica, if any
	#--------
	set len [llength $data(node_daemons)]
	if {$len == 1} { return }
	for {set i 1} {$i <= $len} {incr i} {
		set_vars_in_one_scope $data(runtime_cfg_vars_replica_$i) \
			$data(orb_name).replica_$i
	}
}; # proc process_set_vars_action





#-----------------------------------------------------------------------
# proc process_launch_cmd_action
#-----------------------------------------------------------------------

proc process_launch_cmd_action {} {
	global data

	if {$data(startup_mode) == "disable"} {
		set msg ""; append msg \
			"There is no launch command because the startup " \
			"mode is 'disable'"
		puts stderr [box_msg $msg]
		return
	}

	#--------
	# Remove escape sequences from exe_path and exe_args
	#--------
	regsub -all {\\\\} $data(exe_path) {\\} exe_path
	regsub -all {\\\\} $data(exe_args) {\\} exe_args

	#--------
	# Print the launch command(s) prefixed by the node daemon(s)
	#--------
	if {$data(using_replicas)} {
		set i 1
		set sep ""
		foreach node_d $data(node_daemons) {
			set args [add_orbname_arg $exe_args \
					"$data(orb_name).replica_$i"]
			puts "$sep$node_d: $exe_path $args" 
			incr i
			set sep "\n"
		}
	} else {
		set args [add_orbname_arg $exe_args "$data(orb_name)"]
		puts "$data(node_daemons): $exe_path $args" 
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
if {$action != "create"} {
	read_config_script_and_do_sanity_checks
}
process_${action}_action
