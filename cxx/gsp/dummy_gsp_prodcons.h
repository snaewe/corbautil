//-----------------------------------------------------------------------
// File:	dummy_gsp_prodcons.h
//
// Policy:	ProdCons[PutOp, GetOp, OtherOp]
//
// Description:	The producer-consumer synchronisation policy.
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


#ifndef DUMMY_GSP_PRODCONS_H_
#define DUMMY_GSP_PRODCONS_H_





//--------
// #include's
//--------
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

	int	m_in_critical_section; // Boolean
	long	m_item_count;
};





//--------
// Inline implementation of class GSP_ProdCons
//--------

inline GSP_ProdCons::GSP_ProdCons()
{
	m_in_critical_section = 0;
	m_item_count = 0;
}


inline GSP_ProdCons::~GSP_ProdCons()
{
}





//--------
// Inline implementation of class GSP_ProdCons::PutOp
//--------

inline GSP_ProdCons::PutOp::PutOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;

	m_sync.m_item_count ++;
}


inline GSP_ProdCons::PutOp::~PutOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





//--------
// Inline implementation of class GSP_ProdCons::GetOp
//--------

inline GSP_ProdCons::GetOp::GetOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;

	m_sync.m_item_count --;
	assert(m_sync.m_item_count >= 0);
}


inline GSP_ProdCons::GetOp::~GetOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





//--------
// Inline implementation of class GSP_ProdCons::OtherOp
//--------

inline GSP_ProdCons::OtherOp::OtherOp(GSP_ProdCons &sync_data)
	: m_sync(sync_data)
{
	assert(!m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 1;
}


inline GSP_ProdCons::OtherOp::~OtherOp()
{
	assert(m_sync.m_in_critical_section);
	m_sync.m_in_critical_section = 0;
}





#endif
