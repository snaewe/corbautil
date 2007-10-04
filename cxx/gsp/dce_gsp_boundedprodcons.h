//-----------------------------------------------------------------------
// File:	dce_gsp_boundedprodcons.h
//
// Policy:	BoundedProdCons(int size)[PutOp, GetOp, OtherOp]
//
// Description:	The bounded producer-consumer synchronisation policy.
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


#ifndef DCE_GSP_BOUNDEDPRODCONS_H_
#define DCE_GSP_BOUNDEDPRODCONS_H_





//--------
// #include's
//--------
#include <pthread.h>
#include <assert.h>





//--------
// Forward declarations.
//--------
class GSP_BoundedProdCons;





class GSP_BoundedProdCons {
public:
	inline GSP_BoundedProdCons(int size);
	inline ~GSP_BoundedProdCons();

	class PutOp {
	public:
		inline PutOp(GSP_BoundedProdCons &);
		inline ~PutOp();

	protected:
		GSP_BoundedProdCons	&m_sync;
	};

	class GetOp {
	public:
		inline GetOp(GSP_BoundedProdCons &);
		inline ~GetOp();

	protected:
		GSP_BoundedProdCons	&m_sync;
	};

	class OtherOp {
	public:
		inline OtherOp(GSP_BoundedProdCons &);
		inline ~OtherOp();

	protected:
		GSP_BoundedProdCons	&m_sync;
	};

protected:
	friend class	::GSP_BoundedProdCons::PutOp;
	friend class	::GSP_BoundedProdCons::GetOp;
	friend class	::GSP_BoundedProdCons::OtherOp;

	pthread_mutex_t	m_mutex;
	pthread_cond_t	m_notEmpty;
	pthread_cond_t	m_notFull;
	long		m_item_count;
	long		m_buf_size;
};





//--------
// Inline implementation of class GSP_BoundedProdCons
//--------

inline GSP_BoundedProdCons::GSP_BoundedProdCons(int size)
{
	int status;

	m_item_count = 0;
	m_buf_size = size;

	status = pthread_mutex_init(&m_mutex, pthread_mutexattr_default);
	assert(status == 0);

	status = pthread_cond_init(&this->m_notEmpty, pthread_condattr_default);
	assert(status == 0);

	status = pthread_cond_init(&this->m_notFull, pthread_condattr_default);
	assert(status == 0);
}


inline GSP_BoundedProdCons::~GSP_BoundedProdCons()
{
	int status;

	status = pthread_mutex_destroy(&m_mutex);
	assert(status == 0);

	status = pthread_cond_destroy(&m_notEmpty);
	assert(status == 0);

	status = pthread_cond_destroy(&m_notFull);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_BoundedProdCons::PutOp
//--------

inline GSP_BoundedProdCons::PutOp::PutOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	while (m_sync.m_item_count == m_sync.m_buf_size) {
		status = pthread_cond_wait(&m_sync.m_notFull, &m_sync.m_mutex);
		assert(status == 0);
	}
}


inline GSP_BoundedProdCons::PutOp::~PutOp()
{
	int	status;

	m_sync.m_item_count ++;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);

	status = pthread_cond_signal(&m_sync.m_notEmpty);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_BoundedProdCons::GetOp
//--------

inline GSP_BoundedProdCons::GetOp::GetOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	while (m_sync.m_item_count == 0) {
		status = pthread_cond_wait(&m_sync.m_notEmpty, &m_sync.m_mutex);
		assert(status == 0);
	}
}


inline GSP_BoundedProdCons::GetOp::~GetOp()
{
	int	status;

	m_sync.m_item_count --;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);

	status = pthread_cond_signal(&m_sync.m_notFull);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_BoundedProdCons::OtherOp
//--------

inline GSP_BoundedProdCons::OtherOp::OtherOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);
}


inline GSP_BoundedProdCons::OtherOp::~OtherOp()
{
	int	status;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}





#endif
