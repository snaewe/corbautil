//----------------------------------------------------------------------
// Copyright (c) 2002-2004 IONA Technologies. All rights reserved.
// This software is provided "as is".
//----------------------------------------------------------------------

package com.iona.corbautil;

import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import java.io.*;
import java.util.StringTokenizer;
import java.util.ArrayList;

/**
 * This class makes it easy to import and export CORBA object references
 * via files, the name service or any repository that can be accessed by
 * command-line utilities. The user calls the <code>importObjRef()</code>
 * or <code>exportObjRef()</code> methods with a string containing the
 * instructions.
 *
 * <p><b>Format of export instructions</b><br/>
 *
 * <code>"name_service#path/within/naming/service"</code>.<br/>
 * For example, <code>"name_service#foo/bar"</code>
 *
 * <p><code>"file#path/to/file"</code>.<br/>
 * For example, <code>"file#C:\temp\foo.ior"</code>
 *
 * <p><code>"exec#command with an IOR placeholder"</code>.<br/>
 * For example, <code>"exec#cmd /c echo IOR &gt; foo.ior"</code><br/>
 * Another example, <code>"exec#perl script.pl IOR"</code>
 * <p>
 *
 * <p><code>"java_class#full.package.name.of.a.java.class"</code>.<br/>
 * For example, <code>"java_class#com.iona.corbautil.ImportExportExampleAlgorithm"</code>
 *
 * <p><b>Format of import instructions</b><br/>
 *
 * <code>"name_service#path/within/naming/service"</code>.<br/>
 * For example, <code>"name_service#foo/bar"</code>
 *
 * <p><code>"file#path/to/file"</code>.<br/>
 * For example, <code>"file#C:\temp\foo.ior"</code>
 *
 * <p><code>"exec#command that writes a stringified IOR to standard output"</code>.<br/>
 * For example, <code>"exec#cmd /c type foo.ior"</code><br/>
 * Another example, <code>"exec#perl script.pl"</code>
 * <p>
 *
 * <p><code>"java_class#full.package.name.of.a.java.class"</code>.<br/>
 * For example, <code>"java_class#com.iona.corbautil.ImportExportExampleAlgorithm"</code>
 *
 * <p>Also, any of the URL formats supported by the ORB product are
 * allowed for import (but <i>not</i> for export) instructions.
 * For example:
 * <p><code>"IOR:..."</code><br/>
 *    <code>"corbaloc:..."</code><br/>
 *    <code>"corbaname:..."</code><br/>
 *
 * <p><b>Error handling</b><br/>
 * If any errors occur in <code>importObjRef()</code> or
 * <code>exportObjRef()</code> then they throw an
 * <code>ImportExportException</code> that contains a descriptive
 * error message.
 *
 * <p><b>Example of using <code>importObjRef()</code></b>
 * <pre>
 * import com.iona.corbautil.*;
 * import org.omg.CORBA.*;
 * ...
 * class Client
 * {
 *    static void main(String[] args)
 *    {
 *        org.omg.CORBA.Object tmpObj = null;
 *        Foo                  fooObj = null;
 *
 *        try {
 *            orb    = ORB.init(args, null);
 *            tmpObj = ImportExport.importObjRef(orb, args[1]);
 *            fooObj = FooHelper.narrow(foo);
 *            if (fooObj != null) {
 *                fooObj.someOp();
 *            }
 *        }
 *        catch (ImportExportException ex) {
 *            System.out.println(ex.getMessage());
 *        }
 *        catch (Exception ex) {
 *            System.out.println(ex);
 *        }
 *        orb.destroy();
 *    }
 * }
 * </pre>
 *
 * <p><b>Example of using <code>exportObjRef()</code></b>
 * <pre>
 * import com.iona.corbautil.*;
 * import org.omg.CORBA.*;
 * import org.omg.PortableServer.*;
 * ...
 * class Server
 * {
 *    static void main(String[] args)
 *    {
 *        FooImpl  fooSv = null;
 *
 *        try {
 *            orb   = ORB.init(args, null);
 *            fooSv = ...; // create servant and activate it into a POA
 *            ImportExport.exportObjRef(orb, fooSv._this(), args[1]);
 *            orb.run();
 *        }
 *        catch (ImportExportException ex) {
 *            System.out.println(ex.getMessage());
 *        }
 *        catch (Exception ex) {
 *            System.out.println(ex);
 *        }
 *        orb.destroy();
 *    }
 * }
 * </pre>
 */
public class ImportExport
{

    /**
     * Used to prefix instructions for the CORBA Name Service
     */
    private static final String NS_PREFIX = "name_service#";


    /**
     * Used to prefix instructions for writing to the file system
     */
    private static final String FILE_PREFIX = "file#";


    /**
     * Used to prefix instructions for command execution
     */
    private static final String EXEC_PREFIX = "exec#";


    /**
     * Used to prefix instructions that load a Java classes
     */
    private static final String JAVA_CLASS_PREFIX = "java_class#";

    /**
     * Used to prefix export instructions corbaloc-server functionality
     */
    private static final String CORBALOC_SERVER_PREFIX = "corbaloc_server#";


    /**
     * IOR place holder for use with exec command
     */
    private static final String IOR_PLACE_HOLDER = "IOR";


    /**
     * Initialises the private object reference for the name service,
     * if it hasn't been done already.
     */
    private static NamingContext contactNS(
	org.omg.CORBA.ORB	orb,
	String			nsAddr)
		throws ImportExportException
    {
	NamingContext		nsObj  = null;
	org.omg.CORBA.Object	tmpObj = null;

	if (nsAddr.equals("")) {
		//--------
		// resolve_initial_references()
		//--------
		try {
			tmpObj = orb.resolve_initial_references("NameService");
		}
		catch (Exception ex) {
			throw new ImportExportException(
				  "resolve_initial_references(\"NameService\") "
				+ "failed: "
				+ ex);
		}
	} else {
		tmpObj = importObjRef(orb, nsAddr);
	}

	//--------
	// narrow() to the appropriate type
	//--------
	try {
		nsObj = NamingContextHelper.narrow(tmpObj);
	} catch (Exception ex) {
		throw new ImportExportException(
			"NamingServiceHelper.narrow() failed: " + ex);
	}
	if (nsObj == null) {
		throw new ImportExportException(
			"NamingServiceHelper.narrow() failed: "
			+ "nil object reference");
	}

	return nsObj;
    }


    /**
     * Exports the object reference using the specified instructions.
     *
     * @param orb The orb.
     * @param obj The object
     * @param instructions The instructions
     */
    public static void exportObjRef(
	ORB				orb,
	org.omg.CORBA.Object		obj,
	String				instructions)
		throws ImportExportException
    {
	//--------
	// Complain if we are being asked to export a nil object reference.
	//--------
	if (obj == null) {
		throw new ImportExportException(
			"Attempt to export a nil object reference with "
			+ "export instructions '" + instructions + "'");
	}

	if (instructions.equals("")) {
		return; // don't export the object reference
	}

	//--------
	// Dispatch to the approapriate method, based on the prefix
	// at the start of the instructions.
	//--------
	if (instructions.startsWith(NS_PREFIX)) {
		exportObjRefWithNs(orb, instructions, obj);
	} else if (instructions.startsWith(FILE_PREFIX)) {
		exportObjRefWithFile(orb, obj, instructions);
	} else if (instructions.startsWith(EXEC_PREFIX)) {
	    exportObjRefWithExec(orb, obj, instructions);
	} else if (instructions.startsWith(CORBALOC_SERVER_PREFIX)) {
		exportObjRefWithCorbalocServer(orb,instructions, obj);
	} else if (instructions.startsWith(JAVA_CLASS_PREFIX)) {
		exportObjRefWithJavaClass(orb,instructions, obj);
	} else {
		throw new ImportExportException("Invalid export instructions '"
						+ instructions + "'");
	}
    }


    /**
     * Imports an object reference, using the specified instructions.
     *
     * @param orb The orb.
     * @param instructions The instructions.
     * @return The object reference.
     */
    public static org.omg.CORBA.Object importObjRef(
	ORB				orb,
	String				instructions)
		throws ImportExportException
    {
	org.omg.CORBA.Object result = null;

	//--------
	// Dispatch to the approapriate method, based on the prefix
	// at the start of the instructions.
	//--------
	if (instructions.startsWith(NS_PREFIX)) {
		result = importObjRefWithNs(orb, instructions);
	} else if (instructions.startsWith(FILE_PREFIX)) {
		result = importObjRefWithFile(orb, instructions);
	} else if (instructions.startsWith(EXEC_PREFIX)) {
		result = importObjRefWithExec(orb, instructions);
	} else if (instructions.startsWith(JAVA_CLASS_PREFIX)) {
		result = importObjRefWithJavaClass(orb, instructions);
	} else if (hasUrlPrefix(instructions)) {
		result = importObjRefWithUrl(orb,instructions);
	} else {
		throw new ImportExportException(
			"Invalid import instructions \"" + instructions + "\"");
	}

	//--------
	// Complain if we have imported a nil object reference.
	//--------
	if (result == null) {
		throw new ImportExportException("import instructions '"
			+ instructions + "' produced a nil object reference");
	}

	return result;
    }


    /**
     * Import an object from the name service using the specified instructions.
     *
     * @param orb The orb.
     * @param instructions The instructions.
     * @return The object.
     */
    private static org.omg.CORBA.Object importObjRefWithNs(
	ORB				orb,
	String				instructions)
		throws ImportExportException
    {
	NamingContext			nsObj;
	String				pathInNs;
	String				nsAddr;
	NameComponent[]			nsName;
	org.omg.CORBA.Object		result = null;

	//--------
	// Split "name_service#path-in-ns [@ ns-addr]"
	// into path-in-ns and ns-addr
	//--------
	pathInNs = getPathInNsFromInstructions(instructions);
	nsAddr   = getNsAddressFromInstructions(instructions);

	//--------
	// Contact the Naming Service
	//--------
	try {
		nsObj = contactNS(orb, nsAddr);
	} catch(ImportExportException ex) {
	    throw new ImportExportException(
		  "failed to contact the Naming Service in "
		+ "export instructions '"
		+ instructions + "': " + ex);
	}

	//--------
	// Convert "name_service#path-in-ns" into NameComponent[] format
	//--------
	nsName = NsStringToName(pathInNs);
	if (nsName == null) {
		throw new ImportExportException("invalid name in import "
			+ "instructions '" + instructions + "'");
	}

	//--------
	// resolve() the object from the Naming Service
	//--------
	try {
		result = nsObj.resolve(nsName);
	} catch (java.lang.Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': resolve() failed: " + ex);
	}

	return result;
    }


    /**
     * Writes an object reference to a file.
     *
     * @param orb The orb.
     * @param obj The object.
     * @param instructions The instructions.
     */
    private static void exportObjRefWithFile(
	ORB				orb,
	org.omg.CORBA.Object		obj,
	String				instructions)
		throws ImportExportException
    {
	String				strIOR = null;
	String				fileName = null;

	//--------
	// Stringify the object reference
	//--------
	try {
		strIOR = orb.object_to_string(obj);
	} catch (Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': object_to_string() failed: " + ex);
	}

	//--------
	// Write the stringified object reference to the specified file
	//--------
	fileName = instructions.substring(FILE_PREFIX.length());
	try {
		FileWriter out = new FileWriter(fileName);
		out.write(strIOR);
		out.flush();
		out.close();
	} catch (Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': error writing to file: " + ex);
	}
    }


    /**
     * Reads a stringified object reference from a file.
     *
     * @param orb The orb.
     * @return  obj The object.
     * @param instructions The instructions.
     */
    private static org.omg.CORBA.Object importObjRefWithFile(
	ORB				orb,
	String				instructions)
		throws ImportExportException
    {
	String				fileName;
	BufferedReader			in;
	String				strIOR;
	org.omg.CORBA.Object		result = null;

	//--------
	// Read the first line of the specified file
	//--------
	fileName = instructions.substring(FILE_PREFIX.length());
        try {
		in = new BufferedReader(new FileReader(fileName));
		strIOR = in.readLine();
		in.close();
	}
	catch (Exception ex) {
		throw new ImportExportException("Error reading file in "
			+ "import instructions '" + instructions + "': " + ex);
        }

	//--------
	// Remove any trailing whitespace and unstringify the object reference
	//--------
	strIOR= strIOR.trim();
        try {
		result = orb.string_to_object(strIOR);
	}
	catch (Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': string_to_object() failed: " + ex);
        }

        return result;
    }


    /**
     * Exports the object reference to the CORBA name service using
     * the instructions specified.
     *
     * @param orb The orb.
     * @param instructions The instructions.
     * @param obj The object.
     */
    private static void exportObjRefWithNs(
	ORB				orb,
	String				instructions,
	org.omg.CORBA.Object		obj)
		throws ImportExportException
    {
	NamingContext			nsObj;
	String				pathInNs;
	String				nsAddr;
	NameComponent[]			nsName;

	//--------
	// Split "name_service#path-in-ns [@ ns-addr]"
	// into path-in-ns and ns-addr
	//--------
	pathInNs = getPathInNsFromInstructions(instructions);
	nsAddr   = getNsAddressFromInstructions(instructions);

	//--------
	// Contact the Naming Service
	//--------
	try {
		nsObj = contactNS(orb, nsAddr);
	} catch(ImportExportException ex) {
	    throw new ImportExportException(
		  "failed to contact the Naming Service in "
		+ "export instructions '"
		+ instructions + "': " + ex);
	}

	//--------
	// Convert "name_service#path-in-ns" into NameComponent[] format
	//--------
	nsName = NsStringToName(pathInNs);

	//--------
	// (re)bind the object from the Naming Service
	//--------
	try {
		nsObj.rebind(nsName, obj);
	} catch (Exception ex) {
	    throw new ImportExportException("export failed for instructions '"
		+ instructions + "': rebind() failed: " + ex);
	}
    }


    /**
     * Exports an object reference using the command specified in the
     * instructions string.
     *
     * @param orb The orb.
     * @param obj The object.
     * @param instructions The instructions.
     */
    private static void exportObjRefWithExec(
	ORB				orb,
	org.omg.CORBA.Object		obj,
	String				instructions)
	throws ImportExportException
    {

	String				strIOR = null;
	String				command;
	int				indexOfHolder;
	Process				p;
	
	//--------
	// Stringify the object reference
	//--------
	try {
		strIOR = orb.object_to_string(obj);
	} catch (Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': object_to_string() failed: " + ex);
	}

	//--------
	// Find the IOR placeholder within the command and replace it
	// with the stringified IOR
	//--------
	command = instructions.substring(EXEC_PREFIX.length());
	indexOfHolder = command.indexOf(IOR_PLACE_HOLDER);
	if (indexOfHolder == -1) {
		throw new ImportExportException("invalid export instructions '"
			+ instructions + "': no " + IOR_PLACE_HOLDER
			+ " in command");
	}
	command = command.substring(0, indexOfHolder)
		+ strIOR
		+ command.substring(indexOfHolder + IOR_PLACE_HOLDER.length(),
					command.length());

	//--------
	// Execute the command
	//--------
	try {
		p = Runtime.getRuntime().exec(command);
		p.waitFor();
		int exitStatus = p.exitValue();

		if (exitStatus != 0) {
			throw new ImportExportException("export failed for "
				+ "instructions '" + instructions
				+ "': non-zero exit status");
		}
	} catch (Exception ex) {
		throw new ImportExportException("export failed for "
			+ "instructions '" + instructions
			+ "': non-zero exit status: " + ex);
	}
    }


    /**
     * Imports an object using a command specified in the instructions
     * string.
     *
     * @param orb The orb.
     * @param instructions The instructions.
     * @return The object.
     *
     */
    private static org.omg.CORBA.Object importObjRefWithExec(
	ORB				orb,
	String				instructions)
	throws ImportExportException
    {
	org.omg.CORBA.Object		result = null;
	String				command;
	Process				p;
	BufferedReader			processStdout;
	String				line;
	int				exitStatus;

	try {
		//--------
		// Run the command
		//--------
		command = instructions.substring(EXEC_PREFIX.length());
		p = Runtime.getRuntime().exec(command);
		p.waitFor();

		//--------
		// Read the first line of its standard output
		//--------
		processStdout = new BufferedReader(new InputStreamReader(
							p.getInputStream()));
		line = processStdout.readLine();
		if (line == null) {
			line = "";
		}

	} catch (Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': non-zero exit status: " + ex);
	}

	//--------
	// Check for a normal exit
	//--------
	exitStatus = p.exitValue();
	if (exitStatus != 0) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': non-zero exit status");
	}

	//--------
	// Unstringify the object reference
	//--------
	try {
		result = orb.string_to_object(line);
	} catch (Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + instructions
			+ "': string_to_object() failed: " + ex);
	}

	return result;
    }


    /**
     * Export an object reference using an object that implements the
     * ImportExportAlgorithm interface.
     * <p>
     * For example, implement the interface with a class called
     * MyAlgorithm. Then, specify "java_class#MyAlgorithm" as the
     * instructions: your implementation class will be dynamically
     * loaded and exportObjRef() will be invoked upon it.
     *
     * @param orb The orb.
     * @param obj The object.
     * @param instructions The instructions.
     */
    private static void exportObjRefWithJavaClass(
	ORB				orb,
	String				instructions,
	org.omg.CORBA.Object		obj) throws ImportExportException
    {
	ImportExportAlgorithm		algObj;

	algObj = load(getClassName(instructions), instructions, "export");
	algObj.exportObjRef(orb, obj, instructions);
    }


    /**
     * Export an object reference using an object that implements the
     * ImportExportAlgorithm interface.
     * <p>
     * For example, implement the interface with a class called
     * MyAlgorithm. Then, specify "java_class#MyAlgorithm" as the
     * instructions: your implementation class will be dynamically
     * loaded and exportObjRef() will be invoked upon it.
     *
     * @param orb The orb.
     * @param obj The object.
     * @param instructions The instructions.
     */
    private static void exportObjRefWithCorbalocServer(
	ORB				orb,
	String				instructions,
	org.omg.CORBA.Object		obj) throws ImportExportException
    {
    	String				orbClassName;
	ExportCorbalocServer		exportObj;

	orbClassName = System.getProperty("org.omg.CORBA.ORBClass", "");
	if (orbClassName.equals("com.iona.corba.art.artimpl.ORBImpl")) {
		exportObj = load(
			"com.iona.corbautil.ExportCorbalocServerOrbix",
			instructions);
		exportObj.exportObjRef(orb, obj, instructions);
	} else if (orbClassName.equals("com.ooc.CORBA.ORB")) {
		exportObj = load(
			"com.iona.corbautil.ExportCorbalocServerOrbacus",
			instructions);
		exportObj.exportObjRef(orb, obj, instructions);
	} else {
		throw new ImportExportException(
			"Export failed for instructions '"
			+ instructions
			+ "': corbaloc-server functionality not supported "
			+ "for this CORBA product");
	}
    }


    /**
     * Import an object reference using an object that implements the
     * ImportExportAlgorithm interface.
     * <p>
     * For example, implement the interface with a class called
     * MyAlgorithm. Then, specify "java_class#MyAlgorithm" as the
     * instructions: your implementation class will be dynamically
     * loaded and importObjRef() will be invoked upon it.
     *
     * @param orb The orb.
     * @param instructions The instructions.
     * @return The object.
     */
    private static org.omg.CORBA.Object importObjRefWithJavaClass(
	ORB				orb,
	String				instructions)
		throws ImportExportException
    {
	ImportExportAlgorithm		algObj;
	
	algObj = load(getClassName(instructions), instructions, "import");
	return algObj.importObjRef(orb, instructions);
    }


    /**
     * Extract the classname from an insructions string. The
     * string can be in the form "java_class#<classname> <other args>":
     * this method will return the classname.
     *
     * @param instructions The instructions
     * @return The classname.
     */
    private static String getClassName(String instructions)
    {
	String				afterPrefix;
	String				className;
	int				indexOfSpace;

	afterPrefix = instructions.substring(JAVA_CLASS_PREFIX.length());
	indexOfSpace = afterPrefix.indexOf(' ');
	if (indexOfSpace > 0) {
		className = afterPrefix.substring(0, indexOfSpace);
	} else {
		className = afterPrefix;
	}
	return className;
    }


    /**
     * Dynamically loads an ImportExportAlgorithm implementation
     * from the classpath.
     *
     * @param className The class name.
     * @return An implementation of the ImportExportAlgorithm interface.
     */
    private static ImportExportAlgorithm load(
	String				className,
	String				instructions,
	String				import_or_export)
		throws ImportExportException
    {
	Class				algClass;
	ImportExportAlgorithm		algObj = null;

	try {
		algClass = Class.forName(className);
		algObj = (ImportExportAlgorithm)algClass.newInstance();
	} catch (Exception ex) {
		throw new ImportExportException(
			import_or_export + " failed for instructions '"
			+ instructions
			+ "': cannot create an instance of class '"
			+ className + "': " + ex.getClass().getName());
	}

	return algObj;
    }


    /**
     * Dynamically loads an CorbalocServerExport implementation
     * from the classpath.
     *
     * @param className The class name.
     * @return An implementation of the ImportExportAlgorithm interface.
     */
    private static ExportCorbalocServer load(
	String				className,
	String				instructions)
		throws ImportExportException
    {
	Class				algClass;
	ExportCorbalocServer		algObj = null;

	try {
		algClass = Class.forName(className);
		algObj = (ExportCorbalocServer)algClass.newInstance();
	} catch (Exception ex) {
		throw new ImportExportException(
			"Export failed for instructions '"
			+ instructions
			+ "': cannot create an instance of class '"
			+ className + "': " + ex.getClass().getName());
	}

	return algObj;
    }


    /**
     * Returns true if the instructions string has a URL prefix
     * such as "IOR:", "corbaloc:" or "corbaname:". This method will
     * return true if the string begins with a sequence of letters
     * followed by a colon.
     *
     * @param instructions The instructions string.
     * @return True if the instructions string has a URL prefix.
     */
    private static boolean hasUrlPrefix(String instructions)
    {
	int				i;
	int				len;

	len = instructions.length();
	for (i = 0; i < len; i++) {
		if (instructions.charAt(i) == ':') {
			return true;
		} else if (!Character.isLetter(instructions.charAt(i))) {
			return false;
		}
	}
	return false;
    }


    /**
     * Uses string_to_object() on the string inside in instructions to
     * create a reference to an object.
     *
     * @param orb The orb.
     * @param instructions The instructions, in this case a string beginning
     * with an OMG prefix, for example "IOR:", "corbaloc:" or "corbaname:".
     *
     * @return The object.
     */
    private static org.omg.CORBA.Object importObjRefWithUrl(
	org.omg.CORBA.ORB		orb,
	String				url)
		throws ImportExportException
    {
	org.omg.CORBA.Object		obj = null;

        try {
		obj = orb.string_to_object(url);
	}
	catch (Exception ex) {
		throw new ImportExportException("import failed for "
			+ "instructions '" + url
			+ "': string_to_object() failed: " + ex);
        }
	return obj;
    }


    /**
     * Converts a "path/in/naming/service" string into a sequence
     * of NameCompnent that can then be passed as a parameter when
     * calling bind(), * resolve() and so on.
     *
     * @param str The "path/in/naming/service" string.
     * @return The sequence of NameComponent.
     */
    private static NameComponent[] NsStringToName(String str)
    {
	NameComponent[]		result;
	ArrayList		components;
	ArrayList		idAndKind;
	int			numComponents;
	int			strLen;
	int			i;
	int			dotIndex;
	boolean			isInEsc;

	strLen = str.length();
	if (strLen == 0) {
		return null;
	}

	//--------
	// Split the string into NameComponents, delimited by '/'
	//--------
	components = splitIntoComponents(str, '/');
	if (components == null || components.size() == 0) {
		return null;
	}

	//--------
	// Allocate space for the result
	//--------
	numComponents = components.size();
	result = new NameComponent[numComponents];

	//--------
	// Iterate over all the components, splitting each into
	// its "id" and "kind" fields, and copying into "result".
	//--------
	for (i = 0; i < numComponents; i++) {
		result[i] = new NameComponent();
		idAndKind = splitIntoComponents(
				(String)components.get(i), '.');
		if (idAndKind == null || idAndKind.size() > 2)
		{
			return null;
		}
		result[i].id = removeEscChars((String)idAndKind.get(0));
		if (idAndKind.size() == 2) {
			result[i].kind = removeEscChars(
						(String)idAndKind.get(1));
		} else {
			result[i].kind = "";
		}
	}

    	return result;
    }


    /**
     * Splits the specified string into shorted strings. The splits occur
     * at 'delimiter' characters, unless a delimiter is prefixed with '\\',
     * which is the escape character.
     *
     * @param str        The string to be split
     * @param delimiter  The delimiter character (typically '/' or '.')
     * @return           An ArrayList containing the split strings.
     */
    private static ArrayList splitIntoComponents(String str, char delimiter)
    {
    	ArrayList		result;
	int			i;
	int			startIndex;
	int			strLen;
	boolean			isInEsc;
	char			ch;

	result = new ArrayList();

	if (str.equals("")) {
		result.add("");
		return result;
	}

	//--------
	// Split the string into individual components.
	// Also check that the string does not end with an escape character.
	//--------
	isInEsc = false;
	startIndex = 0;
	strLen = str.length();
	ch = ' ';
	for (i = 0; i < strLen; i++) {
		ch = str.charAt(i);
		if (ch == delimiter) {
			if (isInEsc) {
				isInEsc = false;
			} else {
				result.add(str.substring(startIndex, i));
				startIndex = i + 1;
			}
		} else if (ch == '\\') {
			isInEsc = !isInEsc;
			if (!isInEsc) {
			}
		} else {
			isInEsc = false;
		}
	}
	if (isInEsc) {
		return null;
	}
	if (startIndex < strLen) {
		result.add(str.substring(startIndex));
	} else if (ch == delimiter) {
		result.add("");
	}

	return result;
    }


    /**
     * Removes all occurrances of the escape character ('\\') from the
     * specified string.
     *
     * @param str The string that possibly contains escape characters.
     * @return    A string with all escape characters removed.
     */
    private static String removeEscChars(String str)
    {
	int				i;
	char				ch;
	int				len = str.length();
	StringBuffer			buf = new StringBuffer();
	boolean				isInEsc = false;

	for (i = 0; i < len; i++) {
		ch = str.charAt(i);
		if (isInEsc) {
			buf.append(ch);
			isInEsc = false;
		} else if (ch == '\\') {
			isInEsc = !isInEsc;
		} else {
			buf.append(ch);
		}
	}
	return buf.toString();
    }


    /**
     * Parses import/export instructions of the form
     * "name_service#path-in-ns [@ ns-addr]" and returns the
     * path-in-ns part.
     *
     * @param instructions The import/export instructions.
     * @return The path-in-ns part of the import/export instructions.
     */
    private static String getPathInNsFromInstructions(String instructions)
		throws ImportExportException
    {
	int				start;
	int				len;
	int				i;
	String				result;

	//--------
	// The path_in_ns is terminated by end-of-string or
	// a non-escaped whitespace.
	//--------
	len = instructions.length();
	start = NS_PREFIX.length();
	i = start;
	while (i < len && !Character.isWhitespace(instructions.charAt(i))) {
		if (instructions.charAt(i) == '\\' && i+1 < len) {
			i++;
		}
		i++;
	}

	//--------
	// Make a copy of the string from start to i-1
	//--------
	result = instructions.substring(start, i);
	return result;
    }


    /**
     * Parses import/export instructions of the form
     * "name_service#path-in-ns [@ ns-addr]" and returns the
     * ns-addr part (that is, the bit after '@'), which (recursively)
     * is import instructions.
     *
     * @param instructions The import/export instructions.
     * @return The ns-addr part of the import/export instructions.
     */
    private static String getNsAddressFromInstructions(String instructions)
		throws ImportExportException
    {
	int				start;
	int				len;
	int				i;
	String				result;

	//--------
	// The path_in_ns is terminated by end-of-string or
	// a non-escaped whitespace.
	//--------
	len = instructions.length();
	start = NS_PREFIX.length();
	i = start;
	while (i < len && !Character.isWhitespace(instructions.charAt(i))) {
		if (instructions.charAt(i) == '\\' && i+1 < len) {
			i++;
		}
		i++;
	}

	//--------
	// Skip whitespace before an optional '@'
	//--------
	while (i < len && Character.isWhitespace(instructions.charAt(i))) {
		i++;
	}

	if (i == len) {
		return "";
	}

	if (instructions.charAt(i) != '@') {
		throw new ImportExportException(
			  "Badly formatted import/export instructions '"
			+ instructions
			+ "': was expecting '@' after the "
			+ "path-in-naming-service");
	}

	//--------
	// Skip over '@' and optional whitespace before NS address
	//--------
	i++;
	while (i < len && Character.isWhitespace(instructions.charAt(i))) {
		i++;
	}

	if (i == len) {
		throw new ImportExportException(
			  "Badly formatted import/export instructions '"
			+ instructions
			+ "': was expecting a Naming Service address after the "
			+ "'@'");
	}
	result = instructions.substring(i);
	return result;
    }


}
