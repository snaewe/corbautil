//-----------------------------------------------------------------------
// File:	dummy_gsp_boundedprodcons.h
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


#ifndef DUMMY_GSP_BOUNDEDPRODCONS_H_
#define DUMMY_GSP_BOUNDEDPRODCONS_H_





//--------
// #include's
//--------
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

	int	m_in_critical_section; // Boolean
	long	m_item_count;
	long	m_buf_size;
};





//--------
// Inline implementation of class GSP_BoundedProdCons
//--------

inline GSP_BoundedProdCons::GSP_BoundedProdCons(int size)
{
	m_in_critical_section = 0;
	m_item_count = 0;
	m_buf_size = size;
}


inline GSP_BoundedProdCons::~GSP_BoundedProdCons()
{
}





//--------
// Inline implementation of class GSP_BoundedProdCons::PutOp
//--------

inline GSP_BoundedProdCons::PutOp::PutOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;

	m_sync.m_item_count ++;
	assert(m_sync.m_item_count <= m_sync.m_buf_size);
}


inline GSP_BoundedProdCons::PutOp::~PutOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





//--------
// Inline implementation of class GSP_BoundedProdCons::GetOp
//--------

inline GSP_BoundedProdCons::GetOp::GetOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;

	m_sync.m_item_count --;
	assert(m_sync.m_item_count >= 0);
}


inline GSP_BoundedProdCons::GetOp::~GetOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





//--------
// Inline implementation of class GSP_BoundedProdCons::OtherOp
//--------

inline GSP_BoundedProdCons::OtherOp::OtherOp(GSP_BoundedProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;
}


inline GSP_BoundedProdCons::OtherOp::~OtherOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





#endif
