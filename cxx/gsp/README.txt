The files in this directory implement the Generic Synchronization
Policy (GSP) class library. All the classes are implemented with
"inline" code, so there is no need to compile .cxx files in order
to use this class library.

You should use the -D<symbol_name> option on your C++ compiler
to define one of the following symbols:

	P_USE_WIN32_THREADS
	P_USE_POSIX_THREADS
	P_USE_DCE_THREADS
	P_USE_SOLARIS_THREADS
	P_USE_NO_THREADS

The symbol tells GSP which underlying threading package it should
use.


Author:   Ciaran McHale
Email:    Ciaran@CiaranMcHale.com
Web site: www.CiaranMcHale.com
