//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;
import org.omg.CORBA.*;
import com.ooc.OB.BootManager;
import com.ooc.OB.BootManagerHelper;

/**
 * An interface for exporting object references via Orbacus-proprietary
 * corbaloc-server functionality.
 */

public class ExportCorbalocServerOrbacus
	implements ExportCorbalocServer
{

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
		throws ImportExportException
    {
	String				name;
	org.omg.CORBA.Object		tmpObj;
	BootManager			bm;

	name = instructions.substring("corbaloc_server#".length());
	try {
		tmpObj = orb.resolve_initial_references(
					"BootManager");
		bm = BootManagerHelper.narrow(tmpObj);
		bm.add_binding(name.getBytes(), obj);
	} catch(Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': " + ex);
	}
    }

}
