//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_iostream.h
//
// Description:	A portability wrapper for including <iosteam.h> or
//		<iostream> followed by "using namespace std;"
//
// Note:	The choice between "classic" and "standard" iosteams
//		is determined by whether or not P_USE_OLD_TYPES
//		is defined
//----------------------------------------------------------------------

#ifndef P_IOSTREAM_H_
#define P_IOSTREAM_H_

#if defined(P_USE_OLD_TYPES)
#include <iostream.h>

#else
#include <iostream>
using namespace std;
#endif

#endif /* P_IOSTREAM_H_ */
