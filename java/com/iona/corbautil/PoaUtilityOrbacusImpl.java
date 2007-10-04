//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.CORBA.*;
import java.lang.reflect.*;
import org.omg.PortableServer.POAPackage.*;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAManager;
import com.ooc.OBPortableServer.*;





/**
 * ORBacus-specific class that simplifies the construction of POA hierarchies.
 * An instance of this class is created by calling
 * <code>PoaUtility.init()</code>, subject to system properties having
 * appropriate values.
 */
public class PoaUtilityOrbacusImpl extends PoaUtilityPortableImpl
{

    /**
     * This constructor is not intended to be called by "normal" user code.
     * Instead, it is called by <code>PoaUtility.init()</code>, subject to
     * system properties.
     *
     * @param orb		The ORB.
     */
    public PoaUtilityOrbacusImpl(org.omg.CORBA.ORB orb, int deployModel)
    		throws PoaUtilityException
    {
	super(orb, deployModel);

	//--------
	// Get access to the ORBacus POA Manager Factory
	//--------
	org.omg.CORBA.Object			tmpObj;
	try {
		tmpObj = orb().resolve_initial_references("POAManagerFactory");
		m_poaMgrFactory = POAManagerFactoryHelper.narrow(tmpObj);
	} catch (Exception ex) {
		throw new PoaUtilityException("resolve_initial_references"
			+ "(\"POAManagerFactory\") failed: "
			+ ex);
	}
    }





    /**
     * Create a POA Manager.
     *
     * @param label		A label (name) that is to be associated with
     *				the newly created POA Manager. If one of the
     *				"fixed port" deployment models is used then
     *				ORBacus-specific entries in a runtime
     *				configuration file are used to specify
     *				on which fixed port the POA Manager listens.
     * @return			The newly created POA manager.
     */
    public LabelledPOAManager
    createPoaManager(String label) throws PoaUtilityException
    {
	com.ooc.OBPortableServer.POAManager	poaManager;
	org.omg.PortableServer.POA		tmpPoa;
	Policy[]				tmpPolicies;
	LabelledPOAManager			result;

	result = null;
	if (deployModel() == FIXED_PORTS_NO_IMR
	    || deployModel() == FIXED_PORTS_WITH_IMR)
	{
		firstPoaMgr(false);
		try {
			//--------
			// We do NOT use root().the_POAManager() as the
			// first POA manager. This is because once
			// ORB.init() has been called (which it has by now),
			// there is no way for us to configure the port
			// number on which it should listen.
			//
			// Instead, we always use the Orbacus-proprietary
			// POAManagerFactory to create a POA Manager.
			// However, there is a gotcha. If we are using
			// Orbacus 4.1.x or earlier then we need to
			// invoke
			//
			//     m_poaMgrFactory.create_poa_manager(label)
			//
			// but if we are using Orbacus 4.2 or later then
			// we need to invoke
			//
			//     m_poaMgrFactory.create_POAManager(label,
			//                       new org.omg.CORBA.Policy[0]))
			//
			// Doh! We have to resort to Java's reflection APIs
			// to invoke the correct method.
			//--------
			try {
				poaManager = invoke_create_poa_manager(label);
			} catch(NoSuchMethodException ex) {
				poaManager = invoke_create_POAManager(label);
			}
			result = new LabelledPOAManager(label, poaManager);
		} catch(Exception ex) {
			throw new PoaUtilityException("Error occurred when "
					+ "creating POA Manager \""
					+ label + "\": " + ex.toString());
		}
	} else {
		//--------
		// We are deploying with random ports, so it is safe
		// to use the root POA Manager.
		//--------
		result = super.createPoaManager(label);
	}
	return result;
    }





    /**
     * Create a POA Manager by invoking the pre-4.2 create_poa_manager()
     * method.
     *
     * @param label		A label (name) that is to be associated with
     *				the newly created POA Manager.
     * @return			The newly created POA manager.
     */
    private com.ooc.OBPortableServer.POAManager
    invoke_create_poa_manager(String label) throws Exception
    {
	Method				method;
	Class				c;
	Class[]				pTypes;
	java.lang.Object[]		params;
	java.lang.Object		result;

	c         = m_poaMgrFactory.getClass();
	pTypes    = new Class[1];
	pTypes[0] = label.getClass();
	method    = c.getDeclaredMethod("create_poa_manager", pTypes);
	params    = new java.lang.Object[1];
	params[0] = label;
	result    = method.invoke(m_poaMgrFactory, params);
	return (com.ooc.OBPortableServer.POAManager)result;
    }





    /**
     * Create a POA Manager by invoking the CORBA-compliant
     * create_POAManager() method that has been introduced in Orbacus 4.2.
     *
     * @param label		A label (name) that is to be associated with
     *				the newly created POA Manager.
     * @return			The newly created POA manager.
     */
    private com.ooc.OBPortableServer.POAManager
    invoke_create_POAManager(String label) throws Exception
    {
	Method				method;
	Class				c;
	Class[]				pTypes;
	java.lang.Object[]		params;
	java.lang.Object		result;
	org.omg.CORBA.Policy[]		seq;
	
	
	seq       = new org.omg.CORBA.Policy[0];
	c         = m_poaMgrFactory.getClass();
	pTypes    = new Class[2];
	pTypes[0] = label.getClass();
	pTypes[1] = seq.getClass();
	method    = c.getDeclaredMethod("create_POAManager", pTypes);
	params    = new java.lang.Object[2];
	params[0] = label;
	params[1] = seq;
	result    = method.invoke(m_poaMgrFactory, params);
	return (com.ooc.OBPortableServer.POAManager)result;
    }


    /*
     * ORBacus-proprietary POA Manager factory
     */
    private POAManagerFactory		m_poaMgrFactory;

}
