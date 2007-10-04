//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_orb.h
//
// Description:	A portability wrapper for including a CORBA vendor's
//		header file that defines basic ORB functionality.
//
// Note:	Ensure that one of the following macros is defined
//		before including this file:
//
//			P_USE_ORBIX     (for Orbix)
//			P_USE_ORBABUS   (for Orbacus)
//			P_USE_TAO       (for TAO)
//			P_USE_OMNIORB   (for omniORB)
//----------------------------------------------------------------------

#ifndef P_ORB_H_
#define P_ORB_H_

#if defined(P_USE_ORBIX)
#include <omg/orb.hh>

#elif defined(P_USE_ORBACUS)
#include <OB/CORBA.h>

#elif defined(P_USE_TAO)
#include <tao/corba.h>

#elif defined(P_USE_OMNIORB)
#include <omniORB4/CORBA.h>
#include "p_omniorb_fix.h"

#else
#error "You must #define P_USE_ORBIX, P_USE_ORBACUS, P_USE_TAO or P_USE_OMNIORB"
#endif

#endif /* P_ORB_H_ */
