//-----------------------------------------------------------------------
// File:	dummy_gsp_rw.h
//
// Policy:	RW[ReadOp, WriteOp]	// readers-writer lock
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

#ifndef DUMMY_GSP_RW_H_
#define DUMMY_GSP_RW_H_





//--------
// #include's
//--------
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

	int	m_reader_count;
	int	m_writer_count;
};





//--------
// Inline implementation of class GSP_RW
//--------

inline GSP_RW::GSP_RW()
{
	m_reader_count = 0;
	m_writer_count = 0;
}



inline GSP_RW::~GSP_RW()
{
}





//--------
// Inline implementation of class GSP_RW::ReadOp
//--------

inline GSP_RW::ReadOp::ReadOp(GSP_RW &sync_data)
        : m_sync(sync_data)
{
	assert(m_sync.m_writer_count == 0);
	m_sync.m_reader_count++;
}



inline GSP_RW::ReadOp::~ReadOp()
{
	assert(m_sync.m_writer_count == 0);
	m_sync.m_reader_count--;
}





//--------
// Inline implementation of class GSP_RW::WriteOp
//--------

inline GSP_RW::WriteOp::WriteOp(GSP_RW &sync_data)
        : m_sync(sync_data)
{
	assert(m_sync.m_writer_count == 0);
	assert(m_sync.m_reader_count == 0);
	m_sync.m_writer_count++;
}



inline GSP_RW::WriteOp::~WriteOp()
{
	assert(m_sync.m_writer_count == 1);
	assert(m_sync.m_reader_count == 0);
	m_sync.m_writer_count--;
}





#endif
