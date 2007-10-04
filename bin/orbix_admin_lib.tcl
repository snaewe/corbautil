#-----------------------------------------------------------------------
# Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# File:         orbix_admin_lib.tcl
#
# Description:	A library of Tcl procedures that are used by
#		orbix_srv_admin.tcl and orbix_set_config_vars.tcl
#
# Author:	Ciaran.McHale@iona.com, Principal Consultant
#-----------------------------------------------------------------------





#-----------------------------------------------------------------------
# proc list_orb_hierarchy
#-----------------------------------------------------------------------

proc list_orb_hierarchy {orb_name} {
	set result {}
	set orb_name_list [split $orb_name "."]
	set len [llength $orb_name_list]
	for {set i 0} {$i < $len} {incr i} {
		set sub_list [lrange $orb_name_list 0 $i]
		lappend result [join $sub_list "."]
	}
	return $result
}





#-----------------------------------------------------------------------
# proc does_scope_exist
#-----------------------------------------------------------------------

proc does_scope_exist {scope_name} {
	global		fake_it

	if {[catch {
		scope list $scope_name
	} err]} {
		return 0
	} else {
		return 1
	}
}





#-----------------------------------------------------------------------
# proc list_vars_in_scope
#-----------------------------------------------------------------------

proc list_vars_in_scope {scope} {
	global data errorInfo

	set result {}
	set nested_scope 0
	if {$data(want_execute)} {
		set scope_show_output [scope show $scope]
	} else {
		if {[catch {
			set scope_show_output [scope show $scope]
		} err]} {
			set scope_show_output {}
		}
	}
	foreach line [split $scope_show_output "\n"] {
		set trimmed_line [string trim $line]
		if {$trimmed_line == "\{"} { incr nested_scope; continue }
		if {$trimmed_line == "\}"} { incr nested_scope -1; continue }
		if {$nested_scope != 0 || $trimmed_line == ""} { continue }
		set split_line [split $trimmed_line " "]
		if {[llength $split_line] < 2} { continue }
		set name [lindex $split_line 0]
		lappend result $name
	}
	return $result
}





#-----------------------------------------------------------------------
# proc list_ns_hierarchy
#-----------------------------------------------------------------------

proc list_ns_hierarchy {ns_path} {
	set ns_path [string trim $ns_path]
	set result {}
	set sub_context_list [split $ns_path "/"]
	set len [llength $sub_context_list]
	for {set i 0} {$i < $len} {incr i} {
		set sub_list [lrange $sub_context_list 0 $i]
		lappend result [join $sub_list "/"]
	}
	return $result
}





#-----------------------------------------------------------------------
# proc does_ns_context_exist
#-----------------------------------------------------------------------

proc does_ns_context_exist {context} {
	global fake_it

	if {[catch {
		ns resolve $context
	} err]} {
		set result [info exists fake_it(does_ns_context_exist,$context)]
	} else {
		set result 1
	}
	return $result




}





#-----------------------------------------------------------------------
# proc create_ns_context
#-----------------------------------------------------------------------

proc create_ns_context {name} {
	global data fake_it

	foreach {context} [list_ns_hierarchy $name] {
		if {![does_ns_context_exist $context]} {
			do_command data ns newnc $context
			set fake_it(does_ns_context_exist,$context) 1
		}
	}
}





#-----------------------------------------------------------------------
# proc do_command
#
# Description: Print and/or execute an itadmin command.
#
# Note:	Whether the argument is printed or executed (or both) depends
#	on data(want_diagnostics) and data(want_execute).
#-----------------------------------------------------------------------

proc do_command {_data args} {
	upvar $_data data
	set cmd [join $args " "]
	set cmd ""
	set sep ""
	foreach arg $args {
		if {$arg == "" || [string first " " $arg] != -1} {
			append cmd "$sep\"$arg\""
		} else {
			append cmd "$sep$arg"
		}
		set sep " "
	}
	if {$data(want_diagnostics)} {
		puts "\n$cmd"
	}
	if {$data(want_execute)} {
		if {[catch {
			eval $cmd
		} err]} {
			puts $err
		}
	}
}





#-----------------------------------------------------------------------
# proc box_msg msg
#
# Description: Returns a string that is a boxed, lines-wrapped version
#	      of the specified message (msg). This is used to ensure
#	      that error messages are clearly visible.
#-----------------------------------------------------------------------

proc box_msg {msg} {
	set box_line "********************"; # length = 20
	append box_line $box_line $box_line; # length = 60
	set result "$box_line\n*"
	set line_len 1
	foreach word [split $msg] {
		set len [string length $word]
		if {$line_len + $len + 1 <= 60} {
			append result " $word"
			set line_len [expr $line_len + 1 + $len]
		} else {
			append result "\n* $word"
			set line_len [expr 2 + $len]
		}
	}; # foreach word
	append result "\n$box_line"
	return $result
}





#-----------------------------------------------------------------------
# proc idlgen_getarg
#
# Description:	Analyze the next command-line argument
#-----------------------------------------------------------------------

proc idlgen_getarg {opt_info_list _opt _opt_arg _opt_sym} {
	upvar $_opt opt
	upvar $_opt_arg opt_arg
	upvar $_opt_sym opt_sym
	global argc argv

	if {$argc !=  [llength $argv]} { error "argc/argv mismatch" }
	if {$argc == 0} { error "nothing in argv to parse" }

	set argument [lindex $argv 0]
	foreach opt_info $opt_info_list {
		if {[llength $opt_info] != 3} {
			error "malformed opt_info_list"
		}
		set opt_pattern "^[lindex $opt_info 0]$"
		set opt_has_arg [lindex $opt_info 1]
		set opt_symbol  [lindex $opt_info 2]
		if {[regexp -- $opt_pattern $argument]} {
			set opt     $argument
			set opt_sym $opt_symbol
			if {$opt_has_arg} {
				if {$argc == 1} {
					set opt_arg ""
					set opt_sym "usage"
				} else {
					set opt_arg [lindex $argv 1]
					set argv [lrange $argv 2 end]
					incr argc -2
				}
			} else {
				set opt_arg ""
				set argv [lrange $argv 1 end]
				incr argc -1
			}
			return
		}
	}
	set opt $argument
	set opt_arg ""
	set opt_sym no_match
	set argv [lrange $argv 1 end]
	incr argc -1
	return
}





#-----------------------------------------------------------------------
# proc get_it_dirs
#-----------------------------------------------------------------------

proc get_it_dirs {_array} {
	global tcl_platform env
	upvar $_array	array

	set platform $tcl_platform(platform)

	#--------
	# IT_PRODUCT_DIR
	#--------
	if {[info exist env(IT_PRODUCT_DIR)]} {
		set IT_PRODUCT_DIR $env(IT_PRODUCT_DIR)
	} else {
		if {$platform == "windows"} {
			set IT_PRODUCT_DIR \
				"$env(SYSTEMDRIVE):\\Program Files\\IONA"
		} else {
			set IT_PRODUCT_DIR "/opt/iona"
		}
	}

	#--------
	# IT_CONFIG_DIR
	#--------
	if {[info exist env(IT_CONFIG_DIR)]} {
		set IT_CONFIG_DIR $env(IT_CONFIG_DIR)
	} else {
		set IT_CONFIG_DIR [file nativename $IT_PRODUCT_DIR/etc]
	}

	#--------
	# IT_CONFIG_DOMAINS_DIR
	#--------
	if {[info exist env(IT_CONFIG_DOMAINS_DIR)]} {
		set IT_CONFIG_DOMAINS_DIR $env(IT_CONFIG_DOMAINS_DIR)
	} else {
		if {$platform == "windows"} {
			set IT_CONFIG_DOMAINS_DIR \
					"$IT_PRODUCT_DIR\\etc\\domains"
		} else {
			set IT_CONFIG_DOMAINS_DIR "/etc/opt/iona/domains"
		}
	}

	#--------
	# IT_DOMAIN_NAME
	#--------
	if {[info exist env(IT_DOMAIN_NAME)]} {
		set IT_DOMAIN_NAME $env(IT_DOMAIN_NAME)
	} else {
		set IT_DOMAIN_NAME "default-domain"
	}

	#--------
	# IT_LICENSE_FILE
	#--------
	if {[info exist env(IT_LICENSE_FILE)]} {
		set IT_LICENSE_FILE $env(IT_LICENSE_FILE)
	} else {
		if {$platform == "windows"} {
			set IT_LICENSE_FILE "$IT_PRODUCT_DIR\\etc\\license.txt"
		} else {
			set IT_LICENSE_FILE "/etc/opt/iona/license.txt"
		}
	}

	set array(IT_PRODUCT_DIR) $IT_PRODUCT_DIR
	set array(IT_CONFIG_DIR) $IT_CONFIG_DIR
	set array(IT_CONFIG_DOMAINS_DIR) $IT_CONFIG_DOMAINS_DIR
	set array(IT_DOMAIN_NAME) $IT_DOMAIN_NAME
	set array(IT_LICENSE_FILE) $IT_LICENSE_FILE
}; # proc get_it_dirs





#-----------------------------------------------------------------------
# proc need_ORB_arg
#-----------------------------------------------------------------------

proc need_ORB_arg {arg_name _cmd_line_args} {
	upvar $_cmd_line_args	cmd_line_args
	global data

	if {[lsearch -exact $cmd_line_args $arg_name] != -1} {
		return 0
	}
	set var_name "IT_[string toupper [string range $arg_name 4 end]]"
	foreach entry $data(environment) {
		if {[string match "$var_name=*" $entry]} {
			return 0
		}
	}
	return 1
}; # proc need_ORB_arg





#-----------------------------------------------------------------------
# proc add_orbname_arg
#-----------------------------------------------------------------------

proc add_orbname_arg {args orbname} {
	set args [string trim $args]
	if {$args == ""} {
		set sep ""
	} else {
		set sep " "
	}
	set res "$args$sep-ORBname $orbname"
	if {[string index $res 0] == "-" && [string index $res 1] != "-"} {
		set res "-$res"
	}
	return $res
}





#-----------------------------------------------------------------------
# proc add_ORB_exe_args
#-----------------------------------------------------------------------

proc add_ORB_exe_args {} {
	global data env errorInfo tcl_platform

	get_it_dirs it_dir
	set exe_args $data(exe_args)
	if {$exe_args == ""} { set sep "" } else { set sep " " }

	if {[need_ORB_arg -ORBlicense_file exe_args]} {
		set tmp "-ORBlicense_file $it_dir(IT_LICENSE_FILE)"
		set exe_args "$exe_args$sep$tmp"
		set sep " "
	}

	if {[need_ORB_arg -ORBconfig_domains_dir exe_args]} {
		set tmp "-ORBconfig_domains_dir $it_dir(IT_CONFIG_DOMAINS_DIR)"
		set exe_args "$exe_args$sep$tmp"
		set sep " "
	}

	if {[need_ORB_arg -ORBdomain_name exe_args]} {
		set tmp "-ORBdomain_name $it_dir(IT_DOMAIN_NAME)"
		set exe_args "$exe_args$sep$tmp"
		set sep " "
	}

	set data(exe_args) $exe_args
}; # proc add_ORB_cmd_line_args





#-----------------------------------------------------------------------
# proc escape_special_chars
#-----------------------------------------------------------------------

proc escape_special_chars {_str} {
	upvar $_str	str

	set map [list "\\" "\\\\" "\$" "\\\$" "\[" "\\\[" "\]" "\\\]" \
			"\{" "\\\{"]
	set str [string map $map $str]
}





#-----------------------------------------------------------------------
# proc set_vars_in_one_scope
#-----------------------------------------------------------------------

proc set_vars_in_one_scope {cfg_var_info cfg_scope_name} {
	global data

	foreach name [list_vars_in_scope $cfg_scope_name] {
		set orig_var($name) ""
	}

	#--------
	# Create/modify the runtime configuration variables
	#--------
	foreach {type name value} $cfg_var_info {
		if {[info exist orig_var($name)]} {
			set create_or_modify "modify"
			unset orig_var($name)
		} else {
			set create_or_modify "create"
		}
		do_command data variable $create_or_modify \
			-scope $cfg_scope_name \
			-type $type -value $value $name
	}

	#--------
	# Anything left in orig_var() is a variable that we want to remove
	#--------
	foreach name [array names orig_var] {
		do_command data variable remove -scope $cfg_scope_name $name
	}
}; # proc set_vars_in_one_scope
