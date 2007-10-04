//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.CORBA.*;
import org.omg.PortableServer.*;
import org.omg.PortableServer.POAPackage.*;
import java.util.HashSet;
import java.util.Vector;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.StringTokenizer;
import java.lang.reflect.*;





/**
 * A class that simplifies the construction of POA hierarchies.
 * An instance of this class is created by calling
 * <code>PoaUtility.init()</code>, subject to system properties having
 * appropriate values.
 */
public class PoaUtilityPortableImpl
	extends PoaUtility
{
    /**
     * This constructor is not intended to be called by "normal" user code.
     * Instead, it is called by <code>PoaUtility.init()</code>, subject to
     * system properties.
     *
     * @param orb		The ORB.
     */
    public PoaUtilityPortableImpl(ORB orb, int deployModel)
	    	throws PoaUtilityException
    {
	org.omg.CORBA.Object		tmp_obj;

	m_orb = orb;
	m_firstPoaMgr = true;
	m_deployModel = deployModel;

	try {
		tmp_obj = m_orb.resolve_initial_references("RootPOA");
		m_root = POAHelper.narrow(tmp_obj);
	} catch (Exception ex) {
		throw new PoaUtilityException(
			"resolve_initial_references(\"RootPOA\") "
			+ "failed: " + ex);
	}
    }


    /**
     * Create a POA.
     *
     * @param poaName		Name of the POA being created
     * @param parentPoa		The parent POA
     * @param labelledMgr	The (labelled) POA Manager (previously created
     * 				by calling <code>createPoaManager()</code>
     * @param policiesStr	A string that lists the policies to be used
     *				for the POA, for example,
     *				"persistent + user_id + retain".
     *				Individual policies are spelt just as in the
     *				CORBA specification, except that they are
     *				in lowercase. Any combination of spaces,
     *				commas and/or plus-signs can be used as
     *				separators between policy names.
     * @return			The newly created POA.
     */
    public POA createPoa(
	String			poaName,
	POA			parentPoa,
	LabelledPOAManager	labelledMgr,
	String			policiesStr) throws PoaUtilityException
    {
	Policy[]		policies;
	POA			poa;
	int			len;
	int			i;

	try {
		policies = createPolicyList(labelledMgr.label(),
						policiesStr);
		poa = parentPoa.create_POA(poaName, labelledMgr.mgr(),
								policies);
		len = policies.length;
		for (i = 0; i < len; i++) {
			policies[i].destroy();
		}
	} catch (InvalidPolicy ex) {
		throw new PoaUtilityException(
				"Error occurred when creating POA \""
				+ getFullPoaName(poaName, parentPoa)
				+ "\": InvalidPolicy (index "
				+ ex.index 
				+ " in \"" + policiesStr + "\")");
	} catch(PoaUtilityException ex) {
		throw new PoaUtilityException(
				"Error occurred when creating POA \""
				+ getFullPoaName(poaName, parentPoa)
				+ "\": "
				+ ex.getMessage());
	} catch(Exception ex) {
		throw new PoaUtilityException(
				"Error occurred when creating POA \""
				+ getFullPoaName(poaName, parentPoa)
				+ "\": "
				+ ex);
	}

	return poa;
    }


    /**
     * Returns the full/path/to/POA name of a POA
     *
     * @param localName		The local name of the poa
     * @param parentPoa		The parent POA
     * @return			The fully-scoped POA name in the form
     *				"full/path/to/POA".
     */
    String getFullPoaName(
	String			localName,
	POA			parentPoa) throws PoaUtilityException
    {
	//Vector				vec;
	ArrayList				vec;
	StringBuffer			buf;
	int				len;
	int				i;

	//--------
	// Obtain the hierarchical names of all the ancestors and
	// store them in a Vector
	//--------
	len = 0;
	//vec = new Vector();
	vec = new ArrayList();
	while (parentPoa != m_root) {
		len++;
		vec.add(parentPoa.the_name());
		parentPoa = parentPoa.the_parent();
	}

	//--------
	// Now iterate over the vector backwards to form the
	// name in the correct format.
	//--------
	buf = new StringBuffer();
	for (i = len-1; i >= 0; i--) {
		//buf.append((String)vec.elementAt(i)).append("/");
		buf.append((String)vec.get(i)).append("/");
	}
	buf = buf.append(localName);

	return buf.toString();
    }


    /**
     * Create an array of Policy objects from the stringified list
     *
     * @param poaMgrLabel		The label of the POA Manager
     *					that will control the POA created
     *					with these policy values.
     * @param policiesStr		A stringified list of policy names,
     *					for example
     *					"persistent + user_id + retain".
     *					The name of an individual policy is
     *					identical to the spelling used in the
     *					CORBA specification execept that it
     *					is in lower-case. Any combination of
     *					spaces, command and/or plus-signs can
     *					be used as separators between the
     *					policy names.
     * @return				The created array of Policy objects.
     */
    Policy[]
    createPolicyList(
	String			poa_mgr_label,
	String			policy_list_str) throws PoaUtilityException
    {
	StringTokenizer		tok;
	Policy[]		seq;
	int			i;
	int			len;

	//--------
	// Iterate over a tokenized "policy_list_str", create
	// policy objects and put them into a sequence<Policy>
	//--------
	tok = new StringTokenizer(policy_list_str, " +,");
	len = tok.countTokens();
	seq = new Policy[len];
	for (i = 0; i < len; i++) {
		seq[i] = createPolicy(tok.nextToken(), policy_list_str);
	}

	return seq;
    }


    /**
     * Create a Policy object from the stringified name
     *
     * @param name			The name of a policy, for example,
     *					"persistent", "user_id" or "retain".
     *					The name of an individual policy is
     *					identical to the spelling used in the
     *					CORBA specification execept that it
     *					is in lower-case.
     * @param policiesStr		The full stringified list of policy
     *					names from which the specified name
     *					has been extacted. This parameter is
     *					used only for formating a useful
     *					Exception message if an error occurs.
     * @return				The created Policy object.
     */
    protected Policy
    createPolicy(
	String			name,
	String			policy_list_str) throws PoaUtilityException
    {
	Policy			result;

	if (name.equals("orb_ctrl_model")) {
		result = m_root.create_thread_policy(
			ThreadPolicyValue.ORB_CTRL_MODEL);
	} else if (name.equals("single_thread_model")) {
		result = m_root.create_thread_policy(
			ThreadPolicyValue.SINGLE_THREAD_MODEL);
	} else if (name.equals("transient")) {
		result = m_root.create_lifespan_policy(
			LifespanPolicyValue.TRANSIENT);
	} else if (name.equals("persistent")) {
		result = m_root.create_lifespan_policy(
			LifespanPolicyValue.PERSISTENT);
	} else if (name.equals("unique_id")) {
		result = m_root.create_id_uniqueness_policy(
			IdUniquenessPolicyValue.UNIQUE_ID);
	} else if (name.equals("multiple_id")) {
		result = m_root.create_id_uniqueness_policy(
			IdUniquenessPolicyValue.MULTIPLE_ID);
	} else if (name.equals("user_id")) {
		result = m_root.create_id_assignment_policy(
			IdAssignmentPolicyValue.USER_ID);
	} else if (name.equals("system_id")) {
		result = m_root.create_id_assignment_policy(
			IdAssignmentPolicyValue.SYSTEM_ID);
	} else if (name.equals("implicit_activation")) {
		result = m_root.create_implicit_activation_policy(
			ImplicitActivationPolicyValue.IMPLICIT_ACTIVATION);
	} else if (name.equals("no_implicit_activation")) {
		result = m_root.create_implicit_activation_policy(
			ImplicitActivationPolicyValue.NO_IMPLICIT_ACTIVATION);
	} else if (name.equals("retain")) {
		result = m_root.create_servant_retention_policy(
			ServantRetentionPolicyValue.RETAIN);
	} else if (name.equals("non_retain")) {
		result = m_root.create_servant_retention_policy(
			ServantRetentionPolicyValue.NON_RETAIN);
	} else if (name.equals("use_active_object_map_only")) {
		result = m_root.create_request_processing_policy(
			RequestProcessingPolicyValue
				.USE_ACTIVE_OBJECT_MAP_ONLY);
	} else if (name.equals("use_default_servant")) {
		result = m_root.create_request_processing_policy(
			RequestProcessingPolicyValue.USE_DEFAULT_SERVANT);
	} else if (name.equals("use_servant_manager")) {
		result = m_root.create_request_processing_policy(
			RequestProcessingPolicyValue.USE_SERVANT_MANAGER);
	} else {
		throw new PoaUtilityException(
				"illegal policy name \""
				+ name +
				"\" in list \""
				+ policy_list_str
				+ "\"");
	}
	return result;
    }


    /**
     * Create a POA Manager.
     *
     * @param label		A label (name) that is to be associated with
     *				the newly created POA Manager. The first
     *				time this method is called, it returns the
     *				root POA. Subsequent calls cause it to create
     *				a new POA Manager.
     * @return			The newly created POA manager.
     */
    public LabelledPOAManager
    createPoaManager(String label) throws PoaUtilityException
    {
	LabelledPOAManager		result;

	result = null;
	try {
		if (m_firstPoaMgr) {
			m_firstPoaMgr = false;
			result = new LabelledPOAManager(label,
					m_root.the_POAManager());
		} else {
			POA			tmp_poa;
			Policy[]		tmp_policies;

			//--------
			// We create a POA Manager by creating a
			// (temporary) POA with a nil reference for its
			// POA Manager. When we have what we want we
			// then destroy() the temporary POA.
			//--------
			tmp_policies = createPolicyList(label, "");
			tmp_poa = m_root.create_POA(
					"_tmp_POA_for_POAManager_creation",
					null, tmp_policies);
			result = new LabelledPOAManager(label,
					tmp_poa.the_POAManager());
			tmp_poa.destroy(false, true);
		}
	} catch(PoaUtilityException ex) {
		throw new PoaUtilityException(
				"Error occurred when creating POA Manager \""
				+ label + "\": " + ex.getMessage());
	} catch(Exception ex) {
		throw new PoaUtilityException(
				"Error occurred when creating POA Manager \""
				+ label + "\": " + ex.toString());
	}

	return result;
    }


    /**
     * Accessor for the root POA.
     *
     * @return 		The root POA.
     */
    public POA root()
    {
	return m_root;
    }


    /**
     * Accessor for the ORB.
     *
     * @return 		The ORB.
     */
    public ORB orb()
    {
	return m_orb;
    }


    /**
     * Accessor method. Returns true prior to the first call of
     * <code>createPoaManager()</code>. After the first call of
     * <code>createPoaManager()</code>, this method returns false.
     */
    protected boolean firstPoaMgr()
    {
	return m_firstPoaMgr;
    }

    /**
     * Accessor method.
     */
    protected int deployModel()
    {
	return m_deployModel;
    }


    /**
     * Modifier method for changing the value returned by
     * <code>firstPoaMgr()</code>.
     *
     * @param val		The new value (true or false)
     */
    protected void firstPoaMgr(boolean val)
    {
	m_firstPoaMgr = val;
    }

    /**
     * The ORB to which the created POAs belong.
     */
    protected ORB	m_orb;


    /**
     * The root of the POA hierarchy
     */
    private POA		m_root;


    /**
     * True before the first call to createPoaManager()
     * and false afterwards.
     */
    private boolean	m_firstPoaMgr;

    /**
     * Holds the server's deployment model. One of:
     * <code>PoaUtility.RANDOM_PORTS_NO_IMR</code>,
     * <code>PoaUtility.RANDOM_PORTS_WITH_IMR</code>,
     * <code>PoaUtility.FIXED_PORTS_NO_IMR</code> or
     * <code>PoaUtility.FIXED_PORTS_WITH_IMR</code>.
     */
    private int		m_deployModel;
}
