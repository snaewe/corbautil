//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	PoaUtility.cxx
//
// Description: Class that provides operations to simplify the building
//		of POA hierarchies
//----------------------------------------------------------------------





//--------
// #include's
//--------
#include "PoaUtility.h"
#include "PolicyListParser.h"
#if defined(P_USE_ORBIX) && P_ORBIX_VERSION >= 61
#include <orbix/leasing.hh>
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "p_iostream.h"
#include "p_strstream.h"





namespace corbautil
{


	class PersistentPolicyFinder : public PolicyFactory
	{
	public:
		PersistentPolicyFinder()
		{ m_found_persistent_policy = 0; }
		virtual ~PersistentPolicyFinder() {}

		virtual CORBA::Policy_ptr create(
				CORBA::ORB_ptr	orb,
				const char *	policyName,
				NameValue *	nvArray,
				int		nvArraySize)
		{
			if (strcmp(policyName, "persistent") == 0) {
				m_found_persistent_policy = 1;
			}
			return CORBA::Policy::_nil();
		}

		CORBA::Boolean	found_persistent_policy()
		{ return m_found_persistent_policy; }

	private:
		CORBA::Boolean	m_found_persistent_policy;
	};






//----------------------------------------------------------------------
// Function:	stringToDeploymentModel()
//
// Description:	Convert the (case insensitive) stringified version of
//		a deployment model to the corresponding enum value.
//----------------------------------------------------------------------
PoaUtility::DeploymentModel
PoaUtility::stringToDeploymentModel(const char * str)
						throw(PoaUtilityException)
{
	strstream	buf;
	char *		u_str;
	int		i;
	int		len;
	char		ch;

	len = strlen(str);
	u_str = new char[len+1];
	for (i = 0; i < len; i++) {
		ch = str[i];
		if (islower(ch)) { ch = toupper(ch); }
		u_str[i] = ch;
	}
	u_str[len] = '\0';

	if (strcmp(u_str, "RANDOM_PORTS_NO_IMR") == 0) {
		delete [] u_str;
		return RANDOM_PORTS_NO_IMR;
	}
	else if (strcmp(u_str, "RANDOM_PORTS_WITH_IMR") == 0) {
		delete [] u_str;
		return RANDOM_PORTS_WITH_IMR;
	}
	else if (strcmp(u_str, "FIXED_PORTS_NO_IMR") == 0) {
		delete [] u_str;
		return FIXED_PORTS_NO_IMR;
	}
	else if (strcmp(u_str, "FIXED_PORTS_WITH_IMR") == 0) {
		delete [] u_str;
		return FIXED_PORTS_WITH_IMR;
	}
	buf	<< "Invalid DeploymentModel '" << str << "'" << ends;
	throw PoaUtilityException(buf);
}





//----------------------------------------------------------------------
// Function:	Constructor
//
// Description:	
//----------------------------------------------------------------------

PoaUtility::PoaUtility(CORBA::ORB_ptr orb, DeploymentModel deployModel)
		throw (PoaUtilityException)
{
	CORBA::Object_var		tmpObj;

	m_deployModel   = deployModel;
	m_firstPoaMgr   = 1; // true
	m_orb           = CORBA::ORB::_duplicate(orb);
	m_poa_mgr_count = 1;
	try {
		tmpObj = m_orb->resolve_initial_references("RootPOA");
		m_root = POA::_narrow(tmpObj);
		assert(!CORBA::is_nil(m_root));
	} catch (const CORBA::Exception & ex) {
		strstream	buf;

		buf	<< "resolve_initial_references(\"RootPOA\") failed: "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}

#if defined(P_USE_ORBIX)
	//--------
	// Get access to the Orbix configuration API
	//--------
	try {
		tmpObj = orb->resolve_initial_references("IT_Configuration");
		m_cfg = IT_Config::Configuration::_narrow(tmpObj);
	} catch (const CORBA::Exception & ex) {
		strstream	buf;

		buf	<< "resolve_initial_references(\"IT_Configuration\") "
			<< "failed: "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}
	assert(!CORBA::is_nil(m_cfg));
#endif

#if defined(P_USE_ORBACUS) && P_ORBACUS_VERSION < 420
	//--------
	// Get access to the ORBacus POA Manager Factory
	//--------
	try {
		tmpObj = m_orb->resolve_initial_references("POAManagerFactory");
		m_poaMgrFactory = OBPortableServer::POAManagerFactory::
					_narrow(tmpObj);
	} catch (const CORBA::Exception & ex) {
		strstream	buf;
		buf	<< "resolve_initial_references(\"POAManagerFactory\") "
			<< "failed: "
			<< ex
			<< ends;

		throw PoaUtilityException(buf);
	}
	assert(!CORBA::is_nil(m_poaMgrFactory));
#endif

#if defined(P_USE_ORBACUS) && P_ORBACUS_VERSION >= 420
	//--------
	// Get access to the POA Manager Factory
	//--------
	try {
		m_poaMgrFactory = m_root->the_POAManagerFactory();
	} catch (const CORBA::Exception & ex) {
		strstream	buf;

		buf	<< "rootPOA->the_POAManagerFactory() failed: "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}
	assert(!CORBA::is_nil(m_poaMgrFactory));
#endif
}





//----------------------------------------------------------------------
// Function:	Destructor
//
// Description:	
//----------------------------------------------------------------------

PoaUtility::~PoaUtility()
{
	//--------
	// Nothing to do
	//--------
}





//----------------------------------------------------------------------
// Function:	createPoa()
//
// Description:	
//----------------------------------------------------------------------

POA_ptr
PoaUtility::createPoa(
	const char *		poa_name,
	POA_ptr			parent_poa,
	LabelledPOAManager &	labelled_mgr,
	const char *		policies_str) throw (PoaUtilityException)
{
	CORBA::PolicyList_var	policies;
	POA_var			poa;

	try {
		policies = createPolicyList(labelled_mgr.label(),
					      policies_str);
		poa = parent_poa->create_POA(poa_name, labelled_mgr.mgr(),
							policies.in());
#if !defined(P_USE_ORBIX)
		//--------
		// According to the CORBA specification, you should destroy()
		// all policy objects when you are finished with them.
		// Otherwise, you might leak memory. In reality, most CORBA
		// products do not leak memory in these circumstances.
		// We choose to not destroy() the policy objects if compiling
		// with Orbix to work around a bug associated with the
		// destruction of an Orbix-proprietary lease policy object.
		//--------
		CORBA::ULong len = policies->length();
		for (CORBA::ULong i = 0; i < len; i++) {
			policies[i]->destroy();
		}
#endif
	} catch (PortableServer::POA::InvalidPolicy ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA \""
			<< full_poa_name.in()
			<< "\": InvalidPolicy (index "
			<< ex.index 
			<< " in \""
			<< policies_str
			<< "\")"
			<< ends;
		throw PoaUtilityException(buf);
	} catch(const CORBA::Exception & ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA '"
			<< full_poa_name.in()
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	} catch(const PoaUtilityException & ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA '"
			<< full_poa_name.in()
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}

	return poa._retn();
}





//----------------------------------------------------------------------
// Function:	getFullPoaName()
//
// Description:	
//----------------------------------------------------------------------

char *
PoaUtility::getFullPoaName(
	const char *			local_name,
	POA_ptr				parent_poa)
{
	strstream			buf;
	char *				buf_str;
	char *				result;
	CORBA::StringSeq		seq(5); // pre-allocate some space
	CORBA::ULong			len;
	int				i;

	//--------
	// Obtain the hierarchical names of all the ancestors and
	// store them in a sequence
	//--------
	len = 0;
	seq.length(len);
	while (!parent_poa->_is_equivalent(m_root)) {
		len++;
		seq.length(len);
		seq[len-1] = parent_poa->the_name();
		parent_poa = parent_poa->the_parent();
	}

	//--------
	// Now iterate over the sequence backwards to form the
	// name in the correct format.
	//--------
	for (i = len-1; i >= 0; i--) {
		buf << seq[(CORBA::ULong)(i)] << "/";
	}
	buf << local_name << ends;
	buf_str = buf.str();
	result = CORBA::string_dup(buf_str);
	buf.rdbuf()->freeze(0);
	return result;
}





//----------------------------------------------------------------------
// Function:	createPolicyList()
//
// Description:	
//----------------------------------------------------------------------

CORBA::PolicyList *
PoaUtility::createPolicyList(
	const char *		poa_mgr_label,
	const char *		policy_list_str) throw(PoaUtilityException)
{
	CORBA::PolicyList_var	seq;

	try {
		corbautil::PolicyListParser	parser(m_orb.in());
		seq = parser.parsePolicyList(policy_list_str);

	} catch(const corbautil::ParserException & ex) {
		throw PoaUtilityException(ex.msg);
	}

#if defined(P_USE_ORBIX)
	CORBA::PolicyList_var		dummySeq;
	CORBA::Boolean			has_persistent_policy;
	PersistentPolicyFinder *	factory;

	try {
		//--------
		// Parse the policy list again to see if it has the
		// persistent policy.
		//--------
		factory = new PersistentPolicyFinder();
		corbautil::PolicyListParser  dummyParser(m_orb.in(), factory);
		dummySeq = dummyParser.parsePolicyList(policy_list_str);
		has_persistent_policy = factory->found_persistent_policy();
	} catch(const corbautil::ParserException &) {
		assert(0); // Bug!
	}

	//--------
	// To use fixed port numbers and/or deploy persistent POAs without
	// an IMR in an Orbix application, you have to apply proprietary
	// policies to POAs.
	//--------
	CORBA::ULong seq_len = seq->length();
	if (m_deployModel   == FIXED_PORTS_NO_IMR
	   || m_deployModel == RANDOM_PORTS_NO_IMR)
	{
		CORBA::Any	tmp_any;

		if (has_persistent_policy) {
			tmp_any <<= IT_PortableServer::DIRECT_PERSISTENCE;
			seq_len++;
			seq->length(seq_len);
			seq[seq_len-1] = m_orb->create_policy(
				IT_PortableServer::PERSISTENCE_MODE_POLICY_ID,
				tmp_any);
		}
	}

	if (m_deployModel    == FIXED_PORTS_NO_IMR
	    || m_deployModel == FIXED_PORTS_WITH_IMR)
	{
		CORBA::Any	tmp_any;
		tmp_any <<= CORBA::Any::from_string(
				CORBA::string_dup(poa_mgr_label), 0, 1);
		seq_len++;
		seq->length(seq_len);
		seq[seq_len-1] = m_orb->create_policy(
			IT_CORBA::WELL_KNOWN_ADDRESSING_POLICY_ID,
			tmp_any);
	}
#endif

	return seq._retn();
}





#if defined(P_USE_ORBACUS)
//----------------------------------------------------------------------
// Function:	createPoaManager()
//
// Description:	ORBacus-proprietary implementation of createPoaManager()
//----------------------------------------------------------------------

LabelledPOAManager
PoaUtility::createPoaManager(const char * label) throw (PoaUtilityException)
{
	LabelledPOAManager		result;

	try {
		result.m_label = CORBA::string_dup(label);
		if (m_deployModel    == FIXED_PORTS_NO_IMR
		    || m_deployModel == FIXED_PORTS_WITH_IMR)
		{
			//--------
			// We do NOT use m_root->the_POAManager() as the
			// first POA Manager. This is because once
			// ORB_init() has been called (which it has by now),
			// there is no way for us to configure the port
			// number on which is should listen.
			//
			// Instead, we use the Orbacus-proprietary
			// POAManagerFactory to create a new POA Manager.
			// Up to and including Orbacus 4.1.x, the
			// OB::POAManagerFactory type had operation
			// create_poa_manager(in string id). However,
			// Orbacus 4.2.0 redefines this type to inherit
			// from the (newly defined)
			// PortableServer::POAManagerFactory type, and
			// the signature/spelling of the operation has
			// changed. Hence the need for the #if...#else...endif
			//--------
#if P_ORBACUS_VERSION < 420
			result.m_mgr = m_poaMgrFactory->
						create_poa_manager(label);
#else
			CORBA::PolicyList	seq;
			seq.length(0);
			result.m_mgr = m_poaMgrFactory->
					create_POAManager(label, seq);
#endif
		} else if (m_firstPoaMgr) {
			//--------
			// We are deploying with random ports, so it is
			// safe to use the root POA Manager.
			//--------
			result.m_mgr = m_root->the_POAManager();
		} else {
			CORBA::PolicyList_var	tmp_policies;
			char			poa_name[64]; // Big enogh

			//--------
			// We create a POA Manager by creating a
			// (helper) POA with a nil reference for its
			// POA Manager.
			//--------
			sprintf(poa_name, "_helper_POA_%d", m_poa_mgr_count++);
			tmp_policies = new CORBA::PolicyList(0);
			result.m_helper_poa = m_root->create_POA(poa_name,
					POAManager::_nil(), tmp_policies.in());
			result.m_mgr = result.m_helper_poa->the_POAManager();
		}
	} catch(CORBA::Exception & ex) {
		strstream	buf;

		buf	<< "Error occurred when creating POA Manager '"
			<< label
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}

	m_firstPoaMgr = 0; // false
	return result;
}




#else
//----------------------------------------------------------------------
// Function:	createPoaManager()
//
// Description:	Portable implementation of createPoaManager()
//----------------------------------------------------------------------

LabelledPOAManager
PoaUtility::createPoaManager(const char * label) throw (PoaUtilityException)
{
	LabelledPOAManager		result;
	CORBA::ULong			i;
	CORBA::ULong			len;

	try {
		result.m_label = CORBA::string_dup(label);
		if (m_firstPoaMgr) {
			m_firstPoaMgr = 0; // false
			result.m_mgr = m_root->the_POAManager();
		} else {
			CORBA::PolicyList_var	tmp_policies;
			char			poa_name[64]; // Big enogh

			//--------
			// We create a POA Manager by creating a
			// (helper) POA with a nil reference for its
			// POA Manager.
			//--------
			sprintf(poa_name, "_helper_POA_%d", m_poa_mgr_count++);
			tmp_policies = createPolicyList(result.label(), "");
			result.m_helper_poa = m_root->create_POA(poa_name,
					POAManager::_nil(), tmp_policies.in());
			result.m_mgr = result.m_helper_poa->the_POAManager();
			len = tmp_policies->length();
			for (i = 0; i < len; i++) {
				tmp_policies[i]->destroy();
			}
		}
	} catch(CORBA::Exception & ex) {
		strstream	buf;

		buf	<< "Error occurred when creating POA Manager '"
			<< label
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}

	return result;
}
#endif /* if/else defined(P_USE_ORBACUS) */





#if defined(P_USE_ORBIX)
//----------------------------------------------------------------------
// Function:	createAutoWorkQueue()
//
// Description:	
//----------------------------------------------------------------------

LabelledOrbixWorkQueue
PoaUtility::createAutoWorkQueue(
	const char *				label,
	CORBA::Long				max_size,
	CORBA::ULong				initial_thread_count,
	CORBA::Long				high_water_mark,
	CORBA::Long				low_water_mark,
	CORBA::Long				thread_stack_size_kb)
					throw(PoaUtilityException)
{
	CORBA::Object_var			tmpObj;
	AutomaticWorkQueueFactory_var		factoryObj;
	AutomaticWorkQueue_var			wqObj;
	LabelledOrbixWorkQueue			result;

	//--------
	// Let the specified parameters be overridden by configuration
	// values, if available
	//--------
	max_size = getConfigLong(label, "max_size", max_size);
	initial_thread_count = getConfigLong(label, "initial_thread_count",
						initial_thread_count);
	high_water_mark = getConfigLong(label, "high_water_mark",
						high_water_mark);
	low_water_mark = getConfigLong(label, "low_water_mark", low_water_mark);
	thread_stack_size_kb = getConfigLong(label, "thread_stack_size_kb",
						thread_stack_size_kb);

	try {
		//--------
		// Find the factory
		//--------
		tmpObj = m_orb->resolve_initial_references(
			"IT_AutomaticWorkQueueFactory");
		factoryObj = AutomaticWorkQueueFactory::_narrow(tmpObj);
		assert(!CORBA::is_nil(factoryObj));

		//--------
		// Create the work queue
		//--------
		wqObj = factoryObj->create_work_queue_with_thread_stack_size(
						max_size,
						initial_thread_count,
						high_water_mark,
						low_water_mark,
						thread_stack_size_kb * 1024);
		assert(!CORBA::is_nil(wqObj));

	} catch (const CORBA::Exception &) {
		abort(); // Bug!
	}

	result.m_label   = CORBA::string_dup(label);
	result.m_wq      = wqObj._retn();
	return result;
}





//----------------------------------------------------------------------
// Function:	createManualWorkQueue()
//
// Description:	
//----------------------------------------------------------------------

LabelledOrbixWorkQueue
PoaUtility::createManualWorkQueue(
	const char *				label,
	CORBA::Long				max_size)
				throw(PoaUtilityException)
{
	CORBA::Object_var			tmpObj;
	ManualWorkQueueFactory_var		factoryObj;
	ManualWorkQueue_var			wqObj;
	LabelledOrbixWorkQueue			result;

	//--------
	// Let the specified parameters be overridden by configuration
	// values, if available
	//--------
	max_size = getConfigLong(label, "max_size", max_size);

	try {
		//--------
		// Find the factory
		//--------
		tmpObj = m_orb->resolve_initial_references(
			"IT_ManualWorkQueueFactory");
		factoryObj = ManualWorkQueueFactory::_narrow(tmpObj);
		assert(!CORBA::is_nil(factoryObj));

		//--------
		// Create the work queue
		//--------
		wqObj = factoryObj->create_work_queue(max_size);
		assert(!CORBA::is_nil(wqObj));

	} catch (const CORBA::Exception &) {
		abort(); // Bug!
	}

	result.m_label   = CORBA::string_dup(label);
	result.m_wq      = wqObj._retn();
	return result;
}





//----------------------------------------------------------------------
// Function:	getConfigLong()
//
// Description:	
//----------------------------------------------------------------------

CORBA::Long
PoaUtility::getConfigLong(
		const char *	the_namespace,
		const char *	entry_name,
		CORBA::Long	default_value)
{
	CORBA::String_var		name;
	CORBA::Long			result;

	if (strcmp(the_namespace, "") == 0) {
		return default_value;
	}
	name = CORBA::string_alloc(strlen(the_namespace)+strlen(entry_name)+1);
	sprintf(name.inout(), "%s:%s", the_namespace, entry_name);
	try {
		if (!m_cfg->get_long(name.in(), result)) {
			//--------
			// The entry is missing. Use the default value
			//--------
			result = default_value;
		}
	} catch (const CORBA::Exception &) {
		//--------
		// Something went wrong. Use the default value
		//--------
		result = default_value;
	}
	return result;
}





//----------------------------------------------------------------------
// Function:	createPoa()
//
// Description:	
//----------------------------------------------------------------------

POA_ptr
PoaUtility::createPoa(
	const char *			poa_name,
	POA_ptr				parent_poa,
	LabelledPOAManager &		labelled_mgr,
	const char *			policies_str,
	LabelledOrbixWorkQueue &	labelled_wq)
		throw (PoaUtilityException)
{
	CORBA::PolicyList_var	policies;
	POA_var			poa;
	CORBA::ULong		len;
	CORBA::Any		wq_any;

	try {
		//--------
		// Create the normal policy list
		//--------
		policies = createPolicyList(labelled_mgr.label(),
					      policies_str);

		//--------
		// Now add a work queue policy item to the list
		// Note: the unsigned long constant that we have to
		// pass as a parameter has changed beween ASP 5.1
		// and 6.0.
		//--------
		len = policies->length();
		len ++;
		policies->length(len);
		wq_any <<= labelled_wq.wq();
		policies[len-1] = m_orb->create_policy(
#if P_ORBIX_VERSION < 60
			IT_WorkQueue::WORK_QUEUE_POLICY_ID,
#else
			IT_PortableServer::DISPATCH_WORKQUEUE_POLICY_ID,
#endif
			wq_any);

		//--------
		// Create the POA
		//--------
		poa = parent_poa->create_POA(poa_name, labelled_mgr.mgr(),
							policies.in());
		//--------
		// According to the CORBA specification, you should destroy()
		// all policy objects when you are finished with them.
		// Otherwise, you might leak memory. In reality, most CORBA
		// products do not leak memory in these circumstances.
		// We choose to not destroy() the policy objects if compiling
		// with Orbix to work around a bug associated with the
		// destruction of an Orbix-proprietary lease policy object.
		//--------
#if 0
		len = policies->length();
		for (CORBA::ULong i = 0; i < len; i++) {
			policies[i]->destroy();
		}
#endif
	} catch (PortableServer::POA::InvalidPolicy ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA \""
			<< full_poa_name.in()
			<< "\": InvalidPolicy (index "
			<< ex.index 
			<< " in \""
			<< policies_str
			<< "\")"
			<< ends;
		throw PoaUtilityException(buf);
	} catch(const CORBA::Exception & ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA '"
			<< full_poa_name.in()
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	} catch(const PoaUtilityException & ex) {
		strstream		buf;
		CORBA::String_var	full_poa_name;

		full_poa_name = getFullPoaName(poa_name, parent_poa);
		buf	<< "Error occurred when creating POA '"
			<< full_poa_name.in()
			<< "': "
			<< ex
			<< ends;
		throw PoaUtilityException(buf);
	}

	return poa._retn();
}
#endif /* P_USE_ORBIX */





}; // namespace corbautil
