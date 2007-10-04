//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.CORBA.*;
import org.omg.PortableServer.*;
import java.lang.reflect.*;

/**
 * This class simplifies the construction of POA hierarchies.
 */
abstract public class PoaUtility
{
    /**
     * One of four supported deployment models; the server listens on
     * random ports and is deployed without an IMR.
     */
    public static final int RANDOM_PORTS_NO_IMR = 0;
    /**
     * One of four supported deployment models; the server listens on
     * random ports and is deployed with an IMR.
     */
    public static final int RANDOM_PORTS_WITH_IMR = 1;
    /**
     * One of four supported deployment models; the server listens on
     * fixed ports and is deployed without an IMR.
     */
    public static final int FIXED_PORTS_NO_IMR = 2;
    /**
     * One of four supported deployment models; the server listens on
     * fixed ports and is deployed with an IMR.
     */
    public static final int FIXED_PORTS_WITH_IMR = 3;

    public static int stringToDeploymentModel(String model)
		throws PoaUtilityException
    {
	if (model.equalsIgnoreCase("RANDOM_PORTS_NO_IMR")) {
		return RANDOM_PORTS_NO_IMR;
	} else if (model.equalsIgnoreCase("RANDOM_PORTS_WITH_IMR")) {
		return RANDOM_PORTS_WITH_IMR;
	} else if (model.equalsIgnoreCase("FIXED_PORTS_NO_IMR")) {
		return FIXED_PORTS_NO_IMR;
	} else if (model.equalsIgnoreCase("FIXED_PORTS_WITH_IMR")) {
		return FIXED_PORTS_WITH_IMR;
	}
	throw new PoaUtilityException(
			"Invalid DeploymentModel \"" + model + "\"");
    }

    /**
     * Create an instance of a <code>PoaUtlity</code> class. This class uses
     * system properties combined with Java's reflection APIs to load and
     * create an appropriate sub-class of <code>PoaUtility.</code> The
     * algorithm used is as follows.
     *
     * <p>If the sytem property
     * <code>com.iona.corbautil.PoaUtilityClass</code> exists then the
     * class it specifies is used.<br/>
     * Otherwise, the <code>org.omg.CORBA.ORBClass</code> system
     * property is examined. If this property specifies the class
     * associated with either Orbix or ORBacus then an Orbix-specific or
     * ORBacus-specific subclass of PoaUtility is created.<br/>
     * Otherwise, the <code>com.iona.corbautil.PoaUtilityPortableImpl</code>
     * class is loaded.
     *
     * <p>The <code>com.iona.corbautil.PoaUtilityPortableImpl</code> class
     * is implemented with only CORBA-compliant APIs so it can be used with
     * any CORBA-vendor product. However, it may not be able to use all of
     * the four server deployment models.
     *
     * @param orb		The ORB.
     * @param deployModel	One of the four supported deployment models.
     *				You should set this to one of:
     *				<code>PoaUtility.RANDOM_PORTS_NO_IMR</code>,
     *				<code>PoaUtility.RANDOM_PORTS_WITH_IMR</code>,
     *				<code>PoaUtility.FIXED_PORTS_NO_IMR</code> or
     *				<code>PoaUtility.FIXED_PORTS_WITH_IMR</code>.
     * @return			The newly created POAUtility object
     */
    public static PoaUtility init(ORB orb, int deployModel)
    {
    	String				className;
    	String				orbClassName;
	Class				c;
	Constructor			ctor;
	PoaUtility			result = null;
	
	//--------
	// Determine the name of the PoaUtility implementation class
	//--------
	className = System.getProperty("com.iona.corbautil.PoaUtilityClass");
	if (className == null) {
		orbClassName = System.getProperty("org.omg.CORBA.ORBClass", "");
		if (orbClassName.equals("com.iona.corba.art.artimpl.ORBImpl")) {
			className = "com.iona.corbautil.PoaUtilityOrbixImpl";
		} else if (orbClassName.equals("com.ooc.CORBA.ORB")) {
			className = "com.iona.corbautil.PoaUtilityOrbacusImpl";
		}
		//--------
		// else if ... // for other CORBA implementations
		//--------
		else {
			className = "com.iona.corbautil.PoaUtilityPortableImpl";
		}
	}

	//--------
	// Create an instance of the class by calling its constructor
	// that takes an ORB parameter and a boolean.
	//--------
	try {
		c = Class.forName(className);
		ctor = c.getConstructor(new Class[] {ORB.class, int.class});
		result = (PoaUtility)ctor.newInstance(new java.lang.Object[]
					{orb, new Integer(deployModel)});
	} catch (Exception ex) {
		ex.printStackTrace();
		System.exit(1);
	}
	return result;
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
    abstract public POA createPoa(
	String			poaName,
	POA			parentPoa,
	LabelledPOAManager	labelledMgr,
	String			policiesStr)
		throws PoaUtilityException;


    /**
     * Create a POA Manager.
     *
     * @param label		A label (name) that is to be associated with
     *				the newly created POA Manager. If the
     *				<code>deployModel</code> parameter to
     *				<code>init()</code> specifies a deployment
     *				model that involves fixed ports then
     *				CORBA-vendor-specific entries in a runtime
     *				configuration file and/or command-line options
     *				are used to specify on which fixed port the POA
     *				Manager listens.
     * @return			The newly created POA manager.
     */
    abstract public LabelledPOAManager createPoaManager(String label)
		throws PoaUtilityException;


    /**
     * Accessor for the root POA.
     *
     * @return 		The root POA.
     */
    abstract public POA root();
}
