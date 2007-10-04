#-----------------------------------------------------------------------
# Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
# This software is provided "as is".
#
# If you have installed the CORBA Utilities package into somewhere
# other than C:\corbautil then some path/to/files will need to be
# updated in some files in this "bin" directory. This script performs
# these updates. To run the script, do the following:
#
#	o Make sure that you are in this "bin" directory
#
#	o Run the command "itadmin post_install_fixup.tcl"
#-----------------------------------------------------------------------

set file_list {
	orbix_admin_lib.tcl

	orbix_srv_admin
	orbix_srv_admin.bat
	orbix_srv_admin.tcl

	orbix_set_config_vars
	orbix_set_config_vars.bat
	orbix_set_config_vars.tcl

	orbix_notify_service
	orbix_notify_service.bat
	orbix_notify_service.tcl
	orbix_notify_service_template.des

	orbix_ns_on_fixed_port
	orbix_ns_on_fixed_port.bat
	orbix_ns_on_fixed_port.tcl
}

set pwd [pwd]
foreach file $file_list {
	#--------
	# Read the file's contents
	#--------
	set fd [open $file "r"]
	set contents [read $fd]
	close $fd

	#--------
	# Replace all occurances of "C:/corbautil/bin" with the actual
	# directory.
	#--------
	set map [list "C:/corbautil/bin" [pwd]]
	set contents [string map $map $contents]

	#--------
	# Write the modified contents back to file
	#--------
	set fd [open $file "w"]
	puts -nonewline $fd $contents
	close $fd
}

if {$tcl_platform(platform) == "unix"} {
	set shell_script_list {
		orbix_srv_admin
		orbix_set_config_vars
		orbix_notify_service
		orbix_ns_on_fixed_port
	}
	foreach file $shell_script_list {
		file attributes $file -permissions u+x,g+x,o+x
	}
}
