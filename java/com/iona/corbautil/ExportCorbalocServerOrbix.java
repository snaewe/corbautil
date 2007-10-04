//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;
import org.omg.CORBA.*;
import com.iona.corba.IT_PlainTextKey.Forwarder;
import com.iona.corba.IT_PlainTextKey.ForwarderHelper;

/**
 * An interface for exporting object references via Orbix-proprietary
 * corbaloc-server functionality.
 */

public class ExportCorbalocServerOrbix
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
	org.omg.CORBA.Object		tmpObj;
	Forwarder			forwarder;
	String				name;

	name = instructions.substring("corbaloc_server#".length());
	try {
		tmpObj = orb.resolve_initial_references(
					"IT_PlainTextKeyForwarder");
		forwarder = ForwarderHelper.narrow(tmpObj);
		forwarder.add_plain_text_key(name, obj);
	} catch(Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': " + ex);
	}
    }

}
