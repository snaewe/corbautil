//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;
import org.omg.CORBA.*;

/**
 * Users can implement this interface to develop their own algorithms
 * for importing and exporting object references. For example, implement
 * this interface with a class called <code>com.acme.MyAlgorithm</code>.
 * Then, if you specify <code>"java_class#com.acme.MyAlgorithm"</code> as
 * the instructions, your implementation class will be dynamically loaded
 * and <code>importObjRef()</code> and <code>exportObjRef()</code> will be
 * invoked upon it.
 */

public interface ImportExportAlgorithm {

    /**
     * Exports the object reference using the specified instructions.
     * 
     * @param orb The orb.
     * @param obj The object
     * @param instructions The instructions
     */ 
    public void exportObjRef(
	ORB				orb,
	org.omg.CORBA.Object		obj,
	String				instructions)
		throws ImportExportException;

    /**
     * Imports the object reference using the specified instructions.
     * 
     * @param orb The orb.
     * @param instructions The instructions
     *
     * @return The object.
     */ 
    public org.omg.CORBA.Object importObjRef(
	ORB				orb,
	String				instructions)
		throws ImportExportException;

}
