//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import com.iona.corba.IT_WorkQueue.*;


/**
 * Class that associated a label (name) with an Orbix work queue.
 */
public class LabelledOrbixWorkQueue
{
    /**
     * Create a LabelledOrbixWorkQueue with the specified label and work queue
     *
     * @param label		The label.
     * @param wq		The work queue.
     */
    public LabelledOrbixWorkQueue(String label, WorkQueue wq)
    {
    	m_label = label;
	m_wq    = wq;
    }





    /**
     * Accessor for the label.
     *
     * @return 		The label.
     */
    public String       label()    { return m_label; }





    /**
     * Accessor for the work queue.
     *
     * @return 		The work queue.
     */
    public WorkQueue    wq()       { return m_wq; }





    /**
     * The label
     */
    private String		m_label;





    /**
     * The work queue
     */
    private WorkQueue		m_wq;
}
