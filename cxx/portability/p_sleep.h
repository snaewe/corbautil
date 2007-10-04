//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_sleep.h
//
// Description:	A portability wrapper that provides a sleep(int seconds)
//		function.
//----------------------------------------------------------------------

#ifndef P_SLEEP_H_
#define P_SLEEP_H_

#ifdef WIN32
#include <windows.h>
inline void sleep(int seconds) { ::Sleep(seconds * 1000); }
#else
#include <unistd.h>
#endif

#endif /* P_SLEEP_H_ */
