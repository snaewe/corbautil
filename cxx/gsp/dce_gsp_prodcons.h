//-----------------------------------------------------------------------
// File:	dce_gsp_prodcons.h
//
// Policy:	ProdCons[PutOp, GetOp, OtherOp]
//
// Description:	The producer-consumer synchronisation policy.
//
// Note:	The algorithm is taken from "Programming with Threads"
//		by Kleiman, Shah and Smaalders.
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


#ifndef DCE_GSP_PRODCONS_H_
#define DCE_GSP_PRODCONS_H_





//--------
// #include's
//--------
#include <pthread.h>
#include <assert.h>





//--------
// Forward declarations.
//--------
class GSP_ProdCons;





class GSP_ProdCons {
public:
	inline GSP_ProdCons();
	inline ~GSP_ProdCons();

	class PutOp {
	public:
		inline PutOp(GSP_ProdCons &);
		inline ~PutOp();

	protected:
		GSP_ProdCons	&m_sync;
	};

	class GetOp {
	public:
		inline GetOp(GSP_ProdCons &);
		inline ~GetOp();

	protected:
		GSP_ProdCons	&m_sync;
	};

	class OtherOp {
	public:
		inline OtherOp(GSP_ProdCons &);
		inline ~OtherOp();

	protected:
		GSP_ProdCons	&m_sync;
	};

protected:
	friend class	::GSP_ProdCons::PutOp;
	friend class	::GSP_ProdCons::GetOp;
	friend class	::GSP_ProdCons::OtherOp;

	pthread_mutex_t	m_mutex;
	pthread_cond_t	m_notEmpty;
	long		m_count;
};





//--------
// Inline implementation of class GSP_ProdCons
//--------

inline GSP_ProdCons::GSP_ProdCons()
{
	int status;

	status = pthread_mutex_init(&m_mutex, pthread_mutexattr_default);
	assert(status == 0);

	m_count = 0;

	status = pthread_cond_init(&this->m_notEmpty, pthread_condattr_default);
	assert(status == 0);
}


inline GSP_ProdCons::~GSP_ProdCons()
{
	int status;

	status = pthread_mutex_destroy(&m_mutex);
	assert(status == 0);

	status = pthread_cond_destroy(&m_notEmpty);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_ProdCons::PutOp
//--------

inline GSP_ProdCons::PutOp::PutOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);
}


inline GSP_ProdCons::PutOp::~PutOp()
{
	int	status;

	m_sync.m_count ++;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);

	status = pthread_cond_signal(&m_sync.m_notEmpty);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_ProdCons::GetOp
//--------

inline GSP_ProdCons::GetOp::GetOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	while (m_sync.m_count == 0) {
		status = pthread_cond_wait(&m_sync.m_notEmpty, &m_sync.m_mutex);
		assert(status == 0);
	}
}


inline GSP_ProdCons::GetOp::~GetOp()
{
	int	status;

	m_sync.m_count --;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_ProdCons::OtherOp
//--------

inline GSP_ProdCons::OtherOp::OtherOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);
}


inline GSP_ProdCons::OtherOp::~OtherOp()
{
	int	status;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}





#endif
