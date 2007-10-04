//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.PortableServer.*;


/**
 * This exception class is used to hold exception messages for use with the 
 * <code>PoaUtility</code> class.
 *
 */ 
public class PoaUtilityException
	extends Exception
{
    /**
     * Create an exception with the specified message.
     *
     * @param message The message.
     */
    public PoaUtilityException(String message)
    {
	super(message);
    }
}
