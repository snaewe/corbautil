//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.CORBA.*;
import java.io.*;


/**
 *
 * Example of how to implement the ImportExportAlgorithm interface.
 * To use this implementation, specify
 * "java_class#com.iona.corbautil.ImportExportExampleAlgorithm"
 * as the instructions: this class will be dynamically loaded and used.
 * <p>
 * This class imports object reference by reading them from standard
 * input. It exports object reference by writing them to standard output.
 */

public class ImportExportExampleAlgorithm
	implements ImportExportAlgorithm
{
    /**
     * Writes the IOR and the instructions to the console.
     *
     * @param orb The orb.
     * @param obj The object
     * @param instructions The instructions
     *
     */
    public void exportObjRef(
	ORB				orb,
	org.omg.CORBA.Object		obj,
	String				instructions)
		throws ImportExportException
    {
	String				strIOR = null;
	try {
		strIOR = orb.object_to_string(obj);
	} catch (Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': object_to_string() failed: " + ex);
	}
	System.out.println("instructions = '" + instructions + "'");
	System.out.println("IOR = " + strIOR);
    }

    /**
     * Writes the instructions to the console; then waits for a
     * stringified object references to be typed in. This is
     * unstringified and returned.
     *
     * @param orb The orb.
     * @param instructions The instructions
     *
     * @return The object.
     */
    public org.omg.CORBA.Object importObjRef(
	ORB				orb,
	String				instructions)
		throws ImportExportException
    {
	System.out.println("instructions:   " + instructions);
	System.out.println("Please type in a stringified object reference: ");
	try {
		BufferedReader	stdin = new BufferedReader(
					     new InputStreamReader(System.in));
		String strIOR = stdin.readLine();
		return orb.string_to_object(strIOR);
	} catch (Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': error importing stringified object "
			+ "reference from the console: " + ex);
	}
    }
}
