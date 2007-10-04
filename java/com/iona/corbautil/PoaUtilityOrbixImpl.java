//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import com.iona.corba.IT_CORBA.*;
import com.iona.corba.IT_Config.*;
import com.iona.corba.IT_WorkQueue.*;
import com.iona.corba.IT_PortableServer.*;
import org.omg.PortableServer.POAPackage.*;
import org.omg.CORBA.*;
import org.omg.PortableServer.*;
import java.util.StringTokenizer;
import java.lang.reflect.Field;





/**
 * Orbix-specific class that simplifies the construction of POA hierarchies.
 * An instance of this class is created by calling
 * <code>PoaUtility.init()</code>, subject to system properties having
 * appropriate values.
 */
public class PoaUtilityOrbixImpl extends PoaUtilityPortableImpl
{
    /**
     * This constructor is not intended to be called by "normal" user code.
     * Instead, it is called by <code>PoaUtility.init()</code>, subject to
     * system properties.
     *
     * @param orb		The ORB.
     */
    public PoaUtilityOrbixImpl(org.omg.CORBA.ORB orb, int deployModel)
		throws PoaUtilityException
    {
	super(orb, deployModel);

	//--------
	// Get access to the Orbix configuration API.
	//--------
	try {
		org.omg.CORBA.Object tmpObj
			= orb.resolve_initial_references("IT_Configuration");
		m_cfg = ConfigurationHelper.narrow(tmpObj);
	} catch (Exception ex) {
		throw new PoaUtilityException("resolve_initial_references"
				+ "(\"IT_Configuration\") failed: "
				+ ex);
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
    public POA
    createPoa(
	String			poaName,
	POA			parentPoa,
	LabelledPOAManager	labelledMgr,
	String			policiesStr,
	LabelledOrbixWorkQueue	labelledWq) throws PoaUtilityException
    {
	Policy[]		policies;
	POA			poa;
	int			len;
	int			i;
	Any			wq_any = orb().create_any();

	try {
		//--------
		// Create the normal policy list
		//--------
		policies = createPolicyList(labelledMgr.label(),
							policiesStr);

		//--------
		// Now add a work queue policy item to the list
		//--------
		policies = growPolicySeq(policies, 1);
		len = policies.length;
		WorkQueueHelper.insert(wq_any, labelledWq.wq());
		policies[len-1] = orb().create_policy(
				getWorkQueuePolicyId(), wq_any);

		//--------
		// Create the POA
		//--------
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
			"Error occurred when creating POA '"
			+ getFullPoaName(poaName, parentPoa)
			+ "': "
			+ ex);
	}

	return poa;
    }


    /**
     * Uses Java's reflection APIs to determine if we are using
     * Orbix 6.x or an earlier version and, depending on the answer,
     * returns the value that should be used when creating a Policy
     * object that associates a work queue with a POA.
     */
    private static int getWorkQueuePolicyId()
    {
	Class		c;
	Field		f;
	int		result = 0;

	//--------
	// When associating a work queue with a POA, you have to create a
	// policy, the type of which is specified as an integer parameter to
	// orb.create_policy(). Unfortunately, the value of this integer
	// parameter changed between version Orbix 5.x and Orbix 6.0.
	// Thankfully, the new integer value is specified as an IDL "const"
	// so we can use Java's reflection APIs to ATTEMPT to obtain the
	// Orbix 6.0 value. If the use of reflection fails then we must be
	// using a pre-6.0 version of Orbix so we use the return the older
	// integer value instead.
	//--------
	try {
		c = Class.forName("com.iona.corba.IT_PortableServer"
			+ ".DISPATCH_WORKQUEUE_POLICY_ID");
		f = c.getDeclaredField("value");
		//--------
		// If we get this far then we must be using Orbix 6.0 or newer
		//--------
		result = f.getInt(null);
	} catch(Exception ex) {
		//--------
		// We must be using a pre 6.0 version of Orbix
		//--------
		result = WORK_QUEUE_POLICY_ID.value;
	}
	return result;
    }


    /**
     * Create a labelled Orbix-proprietary work queue.
     *
     * @param label			The label that is to be associated
     *					with the newly created work queue.
     * @param max_size			See Orbix documentation for details.
     * @param initial_thread_count	See Orbix documentation for details.
     * @param high_water_mark		See Orbix documentation for details.
     * @param low_water_mark		See Orbix documentation for details.
     * @return				The newly created automatic work queue.
     */
    public LabelledOrbixWorkQueue
    createAutoWorkQueue(
	String			label,
	int			max_size,
	int			initial_thread_count,
	int			high_water_mark,
	int			low_water_mark) throws PoaUtilityException
    {
	org.omg.CORBA.Object		tmpObj;
	AutomaticWorkQueueFactory	factoryObj;
	AutomaticWorkQueue		wqObj;

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

	wqObj = null;
	try {
		//--------
		// Find the factory
		//--------
		tmpObj = orb().resolve_initial_references(
				"IT_AutomaticWorkQueueFactory");
		factoryObj = AutomaticWorkQueueFactoryHelper.narrow(tmpObj);

		//--------
		// Create the work queue
		//--------
		wqObj = factoryObj.create_work_queue(
						max_size,
						initial_thread_count,
						high_water_mark,
						low_water_mark);

	} catch(Exception ex) {
		throw new PoaUtilityException(
				"Error occurred when creating automatic "
				+ "work queue \""
				+ label
				+ "\": "
				+ ex);
	}
	return new LabelledOrbixWorkQueue(label, wqObj);
    }


    /**
     * Create a labelled Orbix-proprietary work queue.
     *
     * @param label			The label that is to be associated
     *					with the newly created work queue.
     * @param max_size			See Orbix documentation for details.
     * @return				The newly created manual work queue.
     */
    public LabelledOrbixWorkQueue
    createManualWorkQueue(
	String				label,
	int				max_size) throws PoaUtilityException
    {
	org.omg.CORBA.Object		tmpObj;
	ManualWorkQueueFactory		factoryObj;
	ManualWorkQueue			wqObj;

	//--------
	// Let the specified parameters be overridden by configuration
	// values, if available
	//--------
	max_size = getConfigLong(label, "max_size", max_size);

	wqObj = null;
	try {
		//--------
		// Find the factory
		//--------
		tmpObj = orb().resolve_initial_references(
					"IT_ManualWorkQueueFactory");
		factoryObj = ManualWorkQueueFactoryHelper.narrow(tmpObj);

		//--------
		// Create the work queue
		//--------
		wqObj = factoryObj.create_work_queue(max_size);

	} catch(Exception ex) {
		throw new PoaUtilityException(
				"Error occurred when creating manual "
				+ "work queue \""
				+ label
				+ "\": "
				+ ex);
	}
	return new LabelledOrbixWorkQueue(label, wqObj);
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
     *					just like the spelling used in the
     *					CORBA specification execept that it
     *					is in lower-case. Any combination of
     *					spaces, command and/or plus-signs can
     *					be used as separators between the
     *					policy names.
     * @return				The created array of Policy objects.
     */
    protected Policy[]
    createPolicyList(
	String			poaMgrLabel,
	String			policiesStr) throws PoaUtilityException
    {
	boolean			isPersistentPoa;
	StringTokenizer		tok;
	Policy[]		seq;
	Any			tmpAny;
	int			i;
	int			len;
	String			placement = "";

	seq = super.createPolicyList(poaMgrLabel, policiesStr);
	len = seq.length;

	//--------
	// Iterate over a tokenized "policiesStr" to determine
	// if this POA has a PERSISTENT policy
	//--------
	isPersistentPoa = false;
	tok = new StringTokenizer(policiesStr, " +,");
	while (tok.hasMoreTokens()) {
		if (tok.nextToken().equals("persistent")) {
			isPersistentPoa = true;
			break;
		}
	}

	//--------
	// No need to add any proprietary policies for:
	//	RANDOM_PORTS_WITH_IMR, or
	//	RANDOM_PORTS_NO_IMR && !isPersistentPoa
	//--------
	if (deployModel() == RANDOM_PORTS_WITH_IMR) {
		return seq;
	}
	if ((deployModel() == RANDOM_PORTS_NO_IMR) && !isPersistentPoa) {
		return seq;
	}

	//--------
	// Grow the "seq" by 2 if it has a PERSISTENT *and* the
	// deployment model is FIXED_PORTS_NO_IMR. Otherwise grow it by 1.
	//--------
	if ((deployModel() == FIXED_PORTS_NO_IMR) && isPersistentPoa) {
		seq = growPolicySeq(seq, 2);
	} else {
		seq = growPolicySeq(seq, 1);
	}

	try {
		placement = "direct_persistence";
		//--------
		// Add the DIRECT_PERSISTENCE policy, if required
		//--------
		if (isPersistentPoa && (deployModel() != FIXED_PORTS_WITH_IMR))
		{
			tmpAny = orb().create_any();
			PersistenceModePolicyValueHelper.insert(tmpAny,
				PersistenceModePolicyValue.DIRECT_PERSISTENCE);
			seq[len] = orb().create_policy(
				PERSISTENCE_MODE_POLICY_ID.value, tmpAny);
			len ++;
		}

		placement = "well_known_addressing_policy";
		//--------
		// Add the WELL_KNOWN_ADDRESSING_POLICY policy, if required
		//--------
		if (deployModel() != RANDOM_PORTS_NO_IMR) {
			tmpAny = orb().create_any();
			tmpAny.insert_string(poaMgrLabel);
			seq[len] = orb().create_policy(
				WELL_KNOWN_ADDRESSING_POLICY_ID.value, tmpAny);
		}
	} catch (Exception ex) {
		throw new PoaUtilityException(
			"error creating \"" + placement + "\" policy: " + ex);
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
	String			policiesStr) throws PoaUtilityException
    {
	Any			a;
	Policy			result;

	try {
		if (name == "deliver") {
			a = orb().create_any();
			ObjectDeactivationPolicyValueHelper.insert(
				a, ObjectDeactivationPolicyValue.DELIVER);
			return orb().create_policy(
				OBJECT_DEACTIVATION_POLICY_ID.value, a);
		} else if (name == "discard") {
			a = orb().create_any();
			ObjectDeactivationPolicyValueHelper.insert(
				a, ObjectDeactivationPolicyValue.DISCARD);
			return orb().create_policy(
				OBJECT_DEACTIVATION_POLICY_ID.value, a);
		} else if (name == "hold") {
			a = orb().create_any();
			ObjectDeactivationPolicyValueHelper.insert(
				a, ObjectDeactivationPolicyValue.HOLD);
			return orb().create_policy(
				OBJECT_DEACTIVATION_POLICY_ID.value, a);
		}
	} catch (Exception ex) {
		throw new PoaUtilityException(
			"error creating \"" + name + "\" policy: " + ex);
	}
	return super.createPolicy(name, policiesStr);
    }


    /**
     * Returns a new Policy[] that is a copy of "seq" but with space for
     * "numNewElements" more entries at the end.
     *
     * @param seq			The original Policy[] that is to be
     *					copied and increased in size.
     * @param numNewElements		The number of new entries to be
     *					added to the copied array.
     * @return				The new, bigger Policy[].
     */
    private Policy[]
    growPolicySeq(Policy[] seq, int numNewElements)
    {
	Policy[]		newSeq;
	int			len;
	int			i;

	len = seq.length;
	newSeq = new Policy[seq.length + numNewElements];
	for (i = 0; i < len; i++) {
		newSeq[i] = seq[i];
	}
	return newSeq;
    }


    /**
     * Attempts to read the specified integer entry (formed by concatenating
     * <code>namespace + ":" + entryName</code>) from the Orbix runtime
     * configuration file/repository. If the attempt is successful then the
     * obtained value is returned. Otherwise, <code>defaultValue</code> is
     * returned.
     *
     * @param namespace		The namespace to be prefixed to the entry name
     * @param entryName		The name of the configuration variable to be
     *				read.
     * @param defaultValue	The value to be returned if there is an error
     *				reading the specified entry from the runtime
     *				configuration file/repository.
     * @return			Upon success, the obtained value. Upon failure
     *				<code>defaultValue<code>
     */
    private int
    getConfigLong(
	String				namespace,
	String				entryName,
	int				defaultValue)
    {
	String				name;
	int				result = 0;
	IntHolder			result_holder = new IntHolder();

	if (namespace == "") {
		return defaultValue;
	}
	name = namespace + ":" + entryName;
	try {
		if (m_cfg.get_long(name, result_holder)) {
			result = result_holder.value;
		} else {
			//--------
			// The entry is missing. Use the default value
			//--------
			result = defaultValue;
		}
	} catch (Exception ex) {
		//--------
		// Something went wrong. Use the default value
		//--------
		result = defaultValue;
	}
	return result;
    }


    /**
     * The Orbix configuration object.
     */
    private Configuration	m_cfg;
}
