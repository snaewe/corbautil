//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.PortableServer.*;


/**
 * Class that associated a label (name) with a POA Manager.
 * The reason for doing this is that some CORBA products provide
 * proprietary capabilities that allow a POA Manager to listen on
 * a fixed port. Such capabilities typically require that the POA
 * manager has a label (name).
 */
public class LabelledPOAManager
{
    /**
     * Create a LabelledPOAManager with the specified label and POA Manager
     *
     * @param label		The label.
     * @param mgr		The POA Manager.
     */
    public LabelledPOAManager(String label, POAManager mgr)
    {
    	m_label = label;
	m_mgr   = mgr;
    }





    /**
     * Accessor for the label.
     *
     * @return 		The label.
     */
    public String        label()    { return m_label; }





    /**
     * Accessor for the POA Manager.
     *
     * @return 		The POA Manager.
     */
    public POAManager    mgr()      { return m_mgr; }





    /**
     * The label
     */
    private String		m_label;





    /**
     * The POA Manager
     */
    private POAManager		m_mgr;
}
