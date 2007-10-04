//-----------------------------------------------------------------------
// File:	gsp_rw.h
//
// Policy:	RW[ReadOp, WriteOp]	// readers-writer lock
//
// Copyright 2006 Ciaran McHale.
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
//	The above copyright notice and this permission notice shall be
//	included in all copies or substantial portions of the Software.  
// 
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//	NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//	OTHER DEALINGS IN THE SOFTWARE.
//----------------------------------------------------------------------

#ifndef GSP_RW_H_
#define GSP_RW_H_





#if defined(P_USE_WIN32_THREADS)
#	include "win_gsp_rw.h"
#elif defined(P_USE_POSIX_THREADS)
#	include "posix_gsp_rw.h"
#elif defined(P_USE_DCE_THREADS)
#	include "dce_gsp_rw.h"
#elif defined(P_USE_SOLARIS_THREADS)
#	include "sol_gsp_rw.h"
#elif defined(P_USE_NO_THREADS)
#	include "dummy_gsp_rw.h"
#else
#	error "You must #define a P_USE_<platform>_THREADS symbol"
#endif





#endif
