@echo off
REM --------------------------------------------------------------------
REM Copyright IONA Technologies 2002-2005. All rights reserved.
REM This software is provided "as is".
REM --------------------------------------------------------------------
set TCL_SCRIPT=C:/corbautil/bin/orbix_notify_service.tcl
itadmin %TCL_SCRIPT% -exe orbix_notify_service %*