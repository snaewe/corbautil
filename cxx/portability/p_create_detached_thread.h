//-----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	p_create_detached_thread.h
//
// Description:	Utility function to start a detached thread.
//----------------------------------------------------------------------


#ifndef P_CREATE_DETACHED_THREAD_H_
#define P_CREATE_DETACHED_THREAD_H_





#if defined(WIN32)
//----------------------------------------------------------------------
// Windows version
//----------------------------------------------------------------------

#include <process.h>

typedef void *(*util_func_ptr)(void *);

inline void
create_detached_thread(util_func_ptr f, void * arg)
{
	int	tid;
	tid = (int)_beginthread((void(*)(void *))f, 0, (void *)arg);
}
#else /* assume a POSIX system */
//----------------------------------------------------------------------
// POSIX version
//----------------------------------------------------------------------
#include <pthread.h>
#include <assert.h>

typedef void *(*util_func_ptr)(void *);

inline void
create_detached_thread(util_func_ptr f, void * arg)
{
	int			status;
	pthread_t		tid;
	pthread_attr_t		attr;

	pthread_attr_init(&attr);
	status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	assert(status == 0);

	status = pthread_create(&tid, &attr, f, arg);
	assert(status == 0);
}
#endif





#endif /* P_CREATE_DETACHED_THREAD_H_ */
