//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_CosNaming_stub.h
//
// Description:	A portability wrapper for including the stub code
//		header file for CosNaming.idl
//
// Note:	Ensure that one of the following macros is defined
//		before including this file:
//
//			P_USE_ORBIX     (for Orbix)
//			P_USE_ORBABUS   (for Orbacus)
//			P_USE_TAO       (for TAO)
//			P_USE_OMNIORB   (for omniORB)
//----------------------------------------------------------------------

#ifndef P_COSNAMING_STUB_H_
#define P_COSNAMING_STUB_H_

#if defined(P_USE_ORBIX)
#include <omg/CosNaming.hh>

#elif defined(P_USE_ORBACUS)
#include <OB/CORBA.h>
#include <OB/CosNaming.h>

#elif defined(P_USE_TAO)
#include <tao/orbsvcs/orbsvcs/CosNamingC.h>

#elif defined(P_USE_OMNIORB)
#include <omniORB4/CORBA.h>
#include "p_omniorb_fix.h"

#else
#error "You must #define P_USE_ORBIX, P_USE_ORBACUS, P_USE_TAO or P_USE_OMNIORB"
#endif

#endif /* P_COSNAMING_STUB_H_ */
