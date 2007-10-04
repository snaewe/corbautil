//-----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// Public API:	
//	#define P_THREAD_ID_TYPE	...
//	typedef void *(*util_func_ptr)(void *);
//	P_THREAD_ID_TYPE create_joinable_thread(util_func_ptr f, void * arg);
//	void * join_with_thread(P_THREAD_ID_TYPE thread_id);
//
// Portability:
//	You must "#define WIN32" for Windows version.
//	Otherwise you get the POSIX version.
//----------------------------------------------------------------------

#ifndef P_CREATE_JOINABLE_THREAD_H_
#define P_CREATE_JOINABLE_THREAD_H_





//--------
// #include's
//--------
#include "gsp_prodcons.h"





#if defined(WIN32)
//--------
// Windows implementation
//--------
#include <process.h>
#include <stdlib.h>

#define P_THREAD_ID_TYPE	HANDLE

typedef void *(*util_func_ptr)(void *);

struct _help_create_joinable_thread_data {
	util_func_ptr		f;
	void *			arg;
	GSP_ProdCons		sync;
};

static unsigned __stdcall _help_create_joinable_thread(void *p)
{
	util_func_ptr				f;
	void *					arg;
	void *					thread_result;
	_help_create_joinable_thread_data *	data;

	data = (_help_create_joinable_thread_data *)p;
	f   = data->f;
	arg = data->arg;
	{
		GSP_ProdCons::PutOp		scopedLock(data->sync);
	}

	thread_result = f(arg);
	return (unsigned) thread_result;
}

inline P_THREAD_ID_TYPE
create_joinable_thread(util_func_ptr f, void * arg)
{
	_help_create_joinable_thread_data		data;
	HANDLE						thread_handle;
	unsigned					dummy;

	data.f   = f;
	data.arg = arg;
	thread_handle = (HANDLE)_beginthreadex(
					0, // default security
					0, // default stack size
					_help_create_joinable_thread, // func
					&data, // arg
					0, // statup state flag
					&dummy);
	{
		GSP_ProdCons::GetOp		scopedLock(data.sync);
	}
	assert(thread_handle != 0);
	return thread_handle;
}


inline void *
join_with_thread(P_THREAD_ID_TYPE thread_handle)
{
	DWORD					thread_result;

	if (WaitForSingleObject(thread_handle, INFINITE) != WAIT_OBJECT_0) {
		abort();
	}
	if (!GetExitCodeThread(thread_handle, &thread_result)) { abort(); }
	CloseHandle(thread_handle);
	return (void*)thread_result;
}
#else
//--------
// Implementation for POSIX
//--------
#include <pthread.h>
#include <assert.h>

#define P_THREAD_ID_TYPE	pthread_t

typedef void *(*util_func_ptr)(void *);

inline P_THREAD_ID_TYPE
create_joinable_thread(util_func_ptr f, void * arg)
{
	int			status;
	pthread_t		thread_id;

	status = pthread_create(&thread_id, 0, f, arg);
	assert(status == 0);
	return thread_id;
}

inline void *
join_with_thread(P_THREAD_ID_TYPE thread_id)
{
	int			status;
	void *			thread_result;

	status = pthread_join(thread_id, &thread_result);
	assert(status == 0);
	return thread_result;
}
#endif





#endif /* P_CREATE_JOINABLE_THREAD_H_ */
