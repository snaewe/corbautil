//-----------------------------------------------------------------------
// File:	posix_gsp_rw.h
//
// Policy:	RW[ReadOp, WriteOp]	// readers-writer lock
//
// Note:	The algorithm is taken from "Programming with POSIX
//		Threads" by David R. Butenhof
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

#ifndef POSIX_GSP_RW_H_
#define POSIX_GSP_RW_H_





//--------
// #include's
//--------
#include <pthread.h>
#include <assert.h>





//--------
// Forward declarations.
//--------
class GSP_RW;





class GSP_RW {
public:
	inline GSP_RW();
	inline ~GSP_RW();

	class ReadOp {
	public:
		inline ReadOp(GSP_RW &);
		inline ~ReadOp();

	protected:
		GSP_RW      &m_sync;
	};

	class WriteOp {
	public:
		inline WriteOp(GSP_RW &);
		inline ~WriteOp();

	protected:
		GSP_RW      &m_sync;
	};

protected:
	friend  class ::GSP_RW::ReadOp;
	friend  class ::GSP_RW::WriteOp;
	pthread_mutex_t	m_mutex;
	pthread_cond_t	m_read_cond;
	pthread_cond_t	m_write_cond;
	int		m_reader_count;
	int		m_writer_count;		// really a boolean
	int		m_reader_waiting_count;
	int		m_writer_waiting_count;
};





//--------
// Inline implementation of class GSP_RW
//--------

inline GSP_RW::GSP_RW()
{
	int	status;

	m_reader_count         = 0;
	m_writer_count         = 0;
	m_reader_waiting_count = 0;
	m_writer_waiting_count = 0;

	status = pthread_mutex_init(&m_mutex, 0);
	assert(status == 0);

	status = pthread_cond_init(&m_read_cond, 0);
	assert(status == 0);

	status = pthread_cond_init(&m_write_cond, 0);
	assert(status == 0);
}



inline GSP_RW::~GSP_RW()
{
	int	status;

	//--------
	// Sanity checks
	//--------
	status = pthread_mutex_lock(&m_mutex);
	assert(status == 0);
		assert(m_reader_count == 0);
		assert(m_writer_count == 0);
		assert(m_reader_waiting_count == 0);
		assert(m_writer_waiting_count == 0);
	status = pthread_mutex_unlock(&m_mutex);
	assert(status == 0);

	//--------
	// Now destroy everything
	//--------
	status = pthread_mutex_destroy(&m_mutex);
	assert(status == 0);

	status = pthread_cond_destroy(&m_read_cond);
	assert(status == 0);

	status = pthread_cond_destroy(&m_write_cond);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_RW::ReadOp
//--------

inline GSP_RW::ReadOp::ReadOp(GSP_RW &sync_data)
        : m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	if (m_sync.m_writer_count) {
		m_sync.m_reader_waiting_count ++;

		while (m_sync.m_writer_count) {
			status = pthread_cond_wait(&m_sync.m_read_cond,
						   &m_sync.m_mutex);
			assert(status == 0);
		}

		m_sync.m_reader_waiting_count --;
	}

	m_sync.m_reader_count ++;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}



inline GSP_RW::ReadOp::~ReadOp()
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	m_sync.m_reader_count --;

	if (m_sync.m_reader_count == 0 && m_sync.m_writer_waiting_count > 0) {
			status = pthread_cond_signal(&m_sync.m_write_cond);
			assert(status == 0);
	}

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}





//--------
// Inline implementation of class GSP_RW::WriteOp
//--------

inline GSP_RW::WriteOp::WriteOp(GSP_RW &sync_data)
        : m_sync(sync_data)
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	if (m_sync.m_writer_count || m_sync.m_reader_count > 0) {
		m_sync.m_writer_waiting_count ++;
		while (m_sync.m_writer_count || m_sync.m_reader_count > 0) {
			status = pthread_cond_wait(&m_sync.m_write_cond,
						   &m_sync.m_mutex);
			assert(status == 0);
		}
		m_sync.m_writer_waiting_count --;
	}

	assert(m_sync.m_writer_count == 0);
	m_sync.m_writer_count = 1;

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}



inline GSP_RW::WriteOp::~WriteOp()
{
	int	status;

	status = pthread_mutex_lock(&m_sync.m_mutex);
	assert(status == 0);

	assert(m_sync.m_writer_count == 1);
	m_sync.m_writer_count = 0;

	if (m_sync.m_reader_waiting_count > 0) {
		status = pthread_cond_broadcast(&m_sync.m_read_cond);
		assert(status == 0);
	} else if (m_sync.m_writer_waiting_count > 0) {
		status = pthread_cond_signal(&m_sync.m_write_cond);
		assert(status == 0);
	}

	status = pthread_mutex_unlock(&m_sync.m_mutex);
	assert(status == 0);
}





#endif
