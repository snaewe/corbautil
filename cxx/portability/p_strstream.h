//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_strstream.h
//
// Description:	A portability wrapper for including <strsteam.h> or
//		<strstream> followed by "using namespace std;"
//
// Note:	The choice between "classic" and "standard" iosteams
//		is determined by whether or not P_USE_OLD_TYPES
//		is defined
//----------------------------------------------------------------------

#ifndef P_STRSTREAM_H_
#define P_STRSTREAM_H_

#if defined(P_USE_OLD_TYPES)
#ifdef WIN32
#include <strstrea.h>
#else
#include <strstream.h>
#endif

#else
#include <strstream>
using namespace std;
#endif

#endif /* P_STRSTREAM_H_ */
