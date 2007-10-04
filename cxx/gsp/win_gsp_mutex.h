//-----------------------------------------------------------------------
// File:	win_gsp_mutex.h
//
// Policy:	Mutex[Op]	// non-recurisve mutex
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

#ifndef WIN_GSP_MUTEX_H_
#define WIN_GSP_MUTEX_H_





//--------
// #include's
//--------
#include <windows.h>
#include <process.h>
#include <assert.h>





//--------
// Forward declarations.
//--------
class GSP_Mutex;





class GSP_Mutex {
public:
	inline GSP_Mutex();
	inline ~GSP_Mutex();

	class Op {
	public:
		inline Op(GSP_Mutex &);
		inline ~Op();

	protected:
		GSP_Mutex      &m_sync;
	};

protected:
	friend  class ::GSP_Mutex::Op;
	HANDLE	m_mutex;
	int	m_deadlockIfReacquired;
};





//--------
// Inline implementation of class GSP_Mutex
//--------

inline GSP_Mutex::GSP_Mutex()
{
	SECURITY_ATTRIBUTES	attr;

	attr.nLength              = sizeof(SECURITY_ATTRIBUTES);
	attr.lpSecurityDescriptor = NULL;
	attr.bInheritHandle       = TRUE;

	m_mutex = CreateMutex(&attr, FALSE, 0);
	m_deadlockIfReacquired = 0;
}



inline GSP_Mutex::~GSP_Mutex()
{
	CloseHandle(m_mutex);
}





//--------
// Inline implementation of class GSP_Mutex::Op
//--------

inline GSP_Mutex::Op::Op(GSP_Mutex &sync_data)
        : m_sync(sync_data)
{
	WaitForSingleObject(m_sync.m_mutex, INFINITE);
	assert(m_sync.m_deadlockIfReacquired == 0);
	m_sync.m_deadlockIfReacquired = 1;
}



inline GSP_Mutex::Op::~Op()
{
	assert(m_sync.m_deadlockIfReacquired);
	m_sync.m_deadlockIfReacquired = 0;
	ReleaseMutex(m_sync.m_mutex);
}





#endif
