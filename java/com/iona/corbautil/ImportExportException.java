//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

/**
 * This exception class is used to hold exception messages for use with the 
 * <code>ImportExport</code> class.
 *
 */ 
public class ImportExportException 
    extends java.lang.Exception
{
    /**
     * Create an exception with the specified message.
     *
     * @param message The message.
     */
    public ImportExportException(String message)
    {
	super(message);
    }
} 
