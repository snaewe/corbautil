//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_fstream.h
//
// Description:	A portability wrapper for including <fsteam.h> or
//		<fstream> followed by "using namespace std;"
//
// Note:	The choice between "classic" and "standard" iosteams
//		is determined by whether or not P_USE_OLD_TYPES
//		is defined
//----------------------------------------------------------------------

#ifndef P_FSTREAM_H_
#define P_FSTREAM_H_

#if defined(P_USE_OLD_TYPES)
#include <fstream.h>

#else
#include <fstream>
using namespace std;
#endif

#endif /* P_FSTREAM_H_ */
