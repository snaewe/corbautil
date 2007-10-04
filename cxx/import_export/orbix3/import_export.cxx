//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	import_export.cxx
//
// Description: Utility functions for importing and exporting object
//		references.
//
// Format of export instructions
// -----------------------------
//
//	"name_service#<path>"             Example: "name_service#foo/bar"
//	"file#<path/to/file>"             Example: "file#foo.ior"
//	"exec#<cmd with IOR placeholder>" Example: "exec#echo IOR >foo.ior"
//
// Format of import instructions
// -----------------------------
//
//	"name_service#<path>"               Example: "name_service#foo/bar"
//	"file#<path/to/file>"               Example: "file#foo.ior"
//	"exec#<cmd>"                        Example: "exec#cat foo.ior"
//
// Also, any of the "URL" formats supported by the ORB product are
// allowed for import (but NOT export) instructions. For example:
//
//	"IOR:..."
//	"corbaloc:..."
//	"corbaname:..."
//	"file://..."
//
// Error handling
// --------------
// If any errors occur in importObjRef() or exportObjRef() then the
// functions throw a corbautil::ImportExportException.
//
// Example of using exportObjRef()
// ---------------------------------
// ... create "my_obj_ref"
// const char * instructions = argv[1];
// try {
//	corbautil::exportObjRef(orb, my_obj_ref, instructions);
// } catch (const corbautil::ImportExportException & ex) {
//	cerr << ex << endl;
//	orb->destroy();
//	exit(1);
// }
//
// Example of using importObjRef()
// ---------------------------------
// const char * instructions = argv[1];
// CORBA::Object_var obj;
// try {
//	obj = corbautil::importObjRef(orb, instructions);
// } catch (const corbautil::ImportExportException & ex) {
//	cerr << ex << endl;
//	orb->destroy();
//	exit(1);
// }
// ... narrow "obj" to the desired type
//----------------------------------------------------------------------





//--------
// #include's
//--------
#include "import_export.h"
#include <NamingService.hh>
#include <fstream.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
using std::string;





namespace corbautil
{





//--------
// Constant declarations
//--------
static const char *			ns_prefix         = "name_service#";
static const char *			file_prefix       = "file#";
static const char *			exec_prefix       = "exec#";
static const char *			java_class_prefix = "java_class#";
static const char *			ior_placeholder   = "IOR";
#define	MAX_STR_IOR_LEN			10240





//--------
// Forward declarations.
//--------
static void
exportObjRefWithNs(
	CORBA::ORB_ptr		orb,
	const char *		instructions,
	CORBA::Object_ptr	obj) throw(ImportExportException);

static CORBA::Object_ptr
importObjRefWithNs(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException);

static void
exportObjRefWithFile(
	CORBA::ORB_ptr		orb,
	const char *		instructions,
	CORBA::Object_ptr	obj) throw(ImportExportException);

static CORBA::Object_ptr
importObjRefWithFile(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException);

static CORBA::Object_ptr
importObjRefWithUrl(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException);

static void
exportObjRefWithExec(
	CORBA::ORB_ptr		orb,
	const char *		instructions,
	CORBA::Object_ptr	obj) throw(ImportExportException);

static CORBA::Object_ptr
importObjRefWithExec(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException);

static CosNaming::NamingContext_ptr contactNs(
	CORBA::ORB_ptr		orb,
	const char *		ns_addr)
				throw(ImportExportException);

static CosNaming::Name *
NsStringToName(const char * str);

static IT_seq_str *
splitIntoComponents(const char * str, char delimiter);

static char *
removeEscChars(const char * str);

static CORBA::Boolean
hasUrlPrefix(const char * instructions);

static char * getPathInNsFromInstructions(const char * instructions);

static char * getNsAddressFromInstructions(const char * instructions);






void
exportObjRef(
	CORBA::ORB_ptr			orb,
	CORBA::Object_ptr		obj,
	const char *			instructions)
		throw(ImportExportException)
{
	if (CORBA::is_nil(obj)) {
		string msg = string("Attempt to export a nil object ")
			+ "reference with export instructions '"
			+ instructions + "'";
		throw ImportExportException(msg);
	}
	if (instructions[0] == '\0') {
		return; // don't export the object reference
	} else if (strncmp(instructions, ns_prefix, strlen(ns_prefix)) == 0) {
		exportObjRefWithNs(orb, instructions, obj);
	} else if (strncmp(instructions, file_prefix, strlen(file_prefix))==0) {
		exportObjRefWithFile(orb, instructions, obj);
	} else if (strncmp(instructions, exec_prefix, strlen(exec_prefix))==0) {
		exportObjRefWithExec(orb, instructions, obj);
	} else if (strncmp(instructions, java_class_prefix,
		   strlen(java_class_prefix))==0)
	{
		string msg = string("Export instructions of the form '")
			+ java_class_prefix + "...' are not supported by "
			+ "C++ applications: '"
			+ instructions + "'";
		throw ImportExportException(msg);
	} else {
		string msg = string("Invalid export instructions '")
			+ instructions + "'";
		throw ImportExportException(msg);
	}
}





CORBA::Object_ptr
importObjRef(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException)
{
	CORBA::Object_ptr		result;

	result = CORBA::Object::_nil();
	if (strncmp(instructions, ns_prefix, strlen(ns_prefix)) == 0) {
		result = importObjRefWithNs(orb, instructions);
	} else if (strncmp(instructions, file_prefix, strlen(file_prefix))==0) {
		result = importObjRefWithFile(orb, instructions);
	} else if (strncmp(instructions, exec_prefix, strlen(exec_prefix)) == 0) {
		result = importObjRefWithExec(orb, instructions);
	} else if (hasUrlPrefix(instructions)) {
		result = importObjRefWithUrl(orb, instructions);
	} else if (strncmp(instructions, java_class_prefix,
		   strlen(java_class_prefix))==0)
	{
		string msg = string("Import instructions of the form '")
			+ java_class_prefix + "...' are not supported by "
			+ "C++ applications: '"
			+ instructions + "'";
		throw ImportExportException(msg);
	} else {
		string msg = string("Invalid import instructions '")
			+ instructions + "'";
		throw ImportExportException(msg);
	}
	if (CORBA::is_nil(result)) {
		string msg = string("import instructions '")
			+ instructions + "' produced a nil object reference";
		throw ImportExportException(msg);
	}
	return result;
}





static void
exportObjRefWithNs(
	CORBA::ORB_ptr			orb,
	const char *			instructions,
	CORBA::Object_ptr		obj) throw(ImportExportException)
{
	CosNaming::NamingContext_var	ns_obj;
	CosNaming::Name_var		name;
	CORBA::String_var		path_in_ns;
	CORBA::String_var		ns_addr;

	//--------
	// Split "name_service#path_in_ns [@ns_addr]"
	// into path_in_ns and ns_addr
	//--------
	path_in_ns = getPathInNsFromInstructions(instructions);
	ns_addr    = getNsAddressFromInstructions(instructions);

	//--------
	// Contact the Naming Service
	//--------
	try {
		ns_obj = contactNs(orb, ns_addr.in());
	} catch (const ImportExportException & ex) {
		strstream	out;
		out	<< "failed to contact the Naming Service in "
			<< "export instructions '"
			<< instructions
			<< "': "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	//--------
	// Convert "NameService#path-in-ns" into CosNaming::Name format
	//--------
	name = NsStringToName(path_in_ns);
	if (name->length() == 0) {
		string msg = string("Invalid name in export ")
			+ "instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}

	//--------
	// (re)bind the object into the Naming Service
	//--------
	try {
		ns_obj->rebind(name.in(), obj);
	}
	catch (const CORBA::Exception & ex) {
		strstream	out;
		out	<< "export failed for instructions '"
			<< instructions
			<< "': rebind() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}
}





static CORBA::Object_ptr
importObjRefWithNs(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException)
{
	CosNaming::NamingContext_var	ns_obj;
	CosNaming::Name_var		name;
	CORBA::Object_ptr		result;
	CORBA::String_var		path_in_ns;
	CORBA::String_var		ns_addr;

	//--------
	// Split "name_service#path_in_ns [@ns_addr]"
	// into path_in_ns and ns_addr
	//--------
	path_in_ns = getPathInNsFromInstructions(instructions);
	ns_addr    = getNsAddressFromInstructions(instructions);

	//--------
	// Contact the Naming Service
	//--------
	try {
		ns_obj = contactNs(orb, ns_addr.in());
	} catch (const ImportExportException & ex) {
		strstream	out;
		out	<< "failed to contact the Naming Service in "
			<< "import instructions '"
			<< instructions
			<< "': "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	//--------
	// Convert "NameService#path-in-ns" into CosNaming::Name format
	//--------
	name = NsStringToName(path_in_ns);
	if (name->length() == 0) {
		string msg = string("Invalid name in import ")
			+ "instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}

	//--------
	// resolve() the object from the Naming Service
	//--------
	try {
		result = ns_obj->resolve(name.in());
	}
	catch (const CORBA::Exception & ex) {
		strstream	out;
		out	<< "import failed for instructions '"
			<< instructions
			<< "': resolve() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	return result;
}





static void
exportObjRefWithFile(
	CORBA::ORB_ptr			orb,
	const char *			instructions,
	CORBA::Object_ptr		obj) throw(ImportExportException)
{
	CORBA::String_var	str_ior;
	ofstream		out_file(instructions + strlen(file_prefix));

	//--------
	// Open the file
	//--------
	if (!out_file) {
		string msg = string("Cannot open file in export ")
			+ "instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}

	//--------
	// Get a stringified object reference
	//--------
	try {
		str_ior = orb->object_to_string(obj);
	}
	catch (CORBA::Exception & ex) {
		strstream	out;
		out	<< "export failed for instructions '"
			<< instructions
			<< "': object_to_string() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	//--------
	// Write the stringified object reference to the file
	//--------
	out_file << str_ior;
}





static CORBA::Object_ptr
importObjRefWithFile(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException)
{
	CORBA::Object_ptr	obj;
	char			str_ior[MAX_STR_IOR_LEN+1];
	FILE *			in_file;
	char *			fgets_result;
	int			len;

	obj = CORBA::Object::_nil();

	//--------
	// Open the file
	//--------
	in_file = fopen(instructions + strlen(file_prefix), "r");
	if (in_file == 0) {
		string msg = string("Cannot open file in import ")
			+ "instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}

	//--------
	// Read the stringified object reference from the file
	//--------
	str_ior[0] = '\0';
	fgets_result = fgets(str_ior, MAX_STR_IOR_LEN+1, in_file);
	fclose(in_file);
	if (fgets_result != str_ior) {
		string msg = string("Error reading file in import ")
			+ "instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}
	len = strlen(str_ior);
	if (len == MAX_STR_IOR_LEN) {
		string msg = string("Error reading file in import ")
			+ "instructions '" + instructions + "': first line "
			+ "in the file is too long";
		throw ImportExportException(msg);
	}

	//--------
	// Remove any trailing crap, for example, whitespace
	//--------
	while (len > 0 && !isalnum(str_ior[len-1])) {
		str_ior[len-1] = '\0';
		len --;
	}

	//--------
	// Unstringify the object reference
	//--------
	try {
		obj = orb->string_to_object(str_ior);
	}
	catch (CORBA::Exception & ex) {
		strstream	out;
		out	<< "import failed for instructions '"
			<< instructions
			<< "': string_to_object() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	return obj;
}





static CORBA::Object_ptr
importObjRefWithUrl(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException)
{
	CORBA::Object_ptr	obj;

	obj = CORBA::Object::_nil();
	try {
		obj = orb->string_to_object(instructions);
	}
	catch (CORBA::Exception & ex) {
		strstream	out;
		out	<< "import failed for instructions '"
			<< instructions
			<< "': string_to_object() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	return obj;
}





static void
exportObjRefWithExec(
	CORBA::ORB_ptr			orb,
	const char *			instructions,
	CORBA::Object_ptr		obj) throw(ImportExportException)
{
	int				exit_status;
	const char *			orig_cmd;
	const char *			ior_start;
	CORBA::String_var		str_ior;
	CORBA::String_var		cmd;

	//--------
	// Stringify the object reference
	//--------
	try {
		str_ior = orb->object_to_string(obj);
	}
	catch (CORBA::Exception & ex) {
		strstream	out;
		out	<< "export failed for instructions '"
			<< instructions
			<< "': object_to_string() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	//--------
	// Find the start of the IOR placeholder within the command
	//--------
	orig_cmd = instructions + strlen(exec_prefix);
	cmd = CORBA::string_alloc(strlen(orig_cmd) + strlen(str_ior.in()));
	ior_start = strstr(orig_cmd, ior_placeholder);
	if (ior_start == 0) {
		string msg = string("Invalid export instructions '")
			+ instructions + "': no " + ior_placeholder
			+ " in command";
		throw ImportExportException(msg);
	}

	//--------
	// Replace the IOR placeholder with the stringified object reference
	//--------
	strncpy(cmd.inOut(), orig_cmd, ior_start - orig_cmd);
	cmd.inOut()[ior_start-orig_cmd] = '\0';
	strcat(cmd.inOut(), str_ior.in());
	strcat(cmd.inOut(), ior_start + strlen(ior_placeholder));

	//--------
	// Use system() to execute the command
	//--------
	exit_status = system(cmd.in());
	if (exit_status != 0) {
		string msg = string("Export failed for instructions '")
			+ instructions + "': non-zero exit status";
		throw ImportExportException(msg);
	}
}





static CORBA::Object_ptr
importObjRefWithExec(
	CORBA::ORB_ptr		orb,
	const char *		instructions) throw(ImportExportException)
{
	FILE *			file;
	char			str_ior[MAX_STR_IOR_LEN+1];
	int			len;
	CORBA::Object_ptr	obj;

	obj = CORBA::Object::_nil();

	//--------
	// Open a pipe to the the command line
	//--------
#ifdef WIN32
	file = _popen(instructions + strlen(exec_prefix), "r");
#else
	file = popen(instructions + strlen(exec_prefix), "r");
#endif
	if (!file) {
		string msg = string("Error executing command in ")
			+ "import instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}

	//--------
	// Read the stringified object reference from the standard output
	// of the comand.
	//--------
	if (fgets(str_ior, MAX_STR_IOR_LEN, file) != str_ior) {
		string msg = string("Error executing command in ")
			+ "import instructions '" + instructions + "'";
		throw ImportExportException(msg);
	}
	len = strlen(str_ior);
	if (len == MAX_STR_IOR_LEN) {
		string msg = string("Error reading standard output ")
			+ "from import instructions '" + instructions
			+ "': first line in output is too long";
		throw ImportExportException(msg);
	}

	//--------
	// Remove any trailing crap, for example, whitespace
	//--------
	while (len > 0 && !isalnum(str_ior[len-1])) {
		str_ior[len-1] = '\0';
		len --;
	}

	//--------
	// Unstringify the object reference
	//--------
	try {
		obj = orb->string_to_object(str_ior);
	}
	catch (CORBA::Exception & ex) {
		strstream	out;
		out	<< "import failed for instructions '"
			<< instructions
			<< "': string_to_object() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	return obj;
}





static CORBA::Boolean
hasUrlPrefix(const char * instructions)
{
	const char *		p;

	//--------
	// URLs ("IOR:...", "corbaloc:...", "corbaname:..." and so on) all
	// begin with some letters followed by a colon, so that is what
	// we check for.
	//--------
	p = instructions;
	while (isalpha(*p)) {
		p++;
	}
	if (p > instructions && *p == ':') {
		return 1;
	}
	return 0;
}





static CosNaming::NamingContext_ptr
contactNs(
	CORBA::ORB_ptr			orb,
	const char *			ns_addr) throw(ImportExportException)
{
	CosNaming::NamingContext_var	ns_obj;
	CORBA::Object_var		obj;

	if (strcmp(ns_addr, "") == 0) {
		try {
			obj = orb->resolve_initial_references("NameService");
		}
		catch (const CORBA::Exception & ex) {
			strstream	out;
			out	<< "resolve_initial_references"
				<< "(\"NameService\") failed: "
				<< ex
				<< ends;
			throw ImportExportException(out);
		}
	} else {
		obj = importObjRef(orb, ns_addr);
	}

	try {
		ns_obj = CosNaming::NamingContext::_narrow(obj);
	}
	catch (const CORBA::Exception & ex) {
		strstream	out;
		out	<< "CosNaming::NamingContext::_narrow() failed: "
			<< ex
			<< ends;
		throw ImportExportException(out);
	}

	if (CORBA::is_nil(ns_obj)) {
		strstream	out;
		out	<< "CosNaming::NamingContext::_narrow() failed: "
			<< "nil object reference"
			<< ends;
		throw ImportExportException(out);
	}
	return CosNaming::NamingContext::_duplicate(ns_obj);
}





//----------------------------------------------------------------------
// Function:	NsStringToName()
//
// Description:	Convert a string of the form "id.kind/id.kind/..."
//		into CosNaming::Name format for use with the
//		Naming Service. The backslash character ("\") can be
//		used as an escape character within the input string.
//
// Note:	Normal CORBA memory management rules apply so the
//		caller must free the returned sequence.
//
// Note:	If the input string is badly formed then an empty
//		sequence is returned.
//----------------------------------------------------------------------

static CosNaming::Name *
NsStringToName(const char * str)
{
	CosNaming::Name_var		result;
	IT_seq_str_var			components;
	IT_seq_str_var			idAndKind;
	int				numComponents;
	CORBA::ULong			i;

	if (strlen(str) == 0) {
		return new CosNaming::Name(0);
	}

	//--------
	// Split the string into NameComponents, delimited by '/'
	//--------
	components = splitIntoComponents(str, '/');
	if (components->length() == 0) {
		return new CosNaming::Name(0);
	}

	//--------
	// Allocate space for the result
	//--------
	numComponents = components->length();
	result = new CosNaming::Name(numComponents);
	result->length(numComponents);

	//--------
	// Iterate over all the components, splitting each into
	// its "id" and "kind" fields, and copying into "result".
	//--------
	for (i = 0; i < numComponents; i++) {
		idAndKind = splitIntoComponents(components[i].in(), '.');
		if (idAndKind->length() == 0 || idAndKind->length() > 2) {
			return new CosNaming::Name(0);
		}
		result[i].id = removeEscChars(
					idAndKind[(CORBA::ULong)0].in());
		if (idAndKind->length() == 2) {
			result[i].kind = removeEscChars(
					idAndKind[(CORBA::ULong)1].in());
		} else {
			result[i].kind = CORBA::string_dup("");
		}
	}

    	return new CosNaming::Name(result.in());
}





static IT_seq_str *
splitIntoComponents(const char * str, char delimiter)
{
	IT_seq_str_var		result;
	CORBA::ULong		i;
	CORBA::ULong		startIndex;
	int			len;
	CORBA::ULong		resultLen;
	CORBA::Boolean		isInEsc;
	char			ch;
	char *			tmpStr;

	result = new IT_seq_str(10);
	resultLen = 0;
	if (strcmp(str, "") == 0) {
		resultLen ++;
		result->length(resultLen);
		result[resultLen-1] = CORBA::string_dup("");
		return new IT_seq_str(result.in());
	}

	//--------
	// Split the string into individual components.
	// Also check that the string does not end with an escape character.
	//--------
	isInEsc = 0; // false
	startIndex = 0;
	len = strlen(str);
	ch = ' ';
	for (i = 0; i < len; i++) {
		ch = str[i];
		if (ch == delimiter) {
			if (isInEsc) {
				isInEsc = 0; // false
			} else {
				resultLen ++;
				result->length(resultLen);
				tmpStr = CORBA::string_dup(&str[startIndex]);
				tmpStr[i - startIndex] = '\0';
				result[(CORBA::ULong)(resultLen-1)] = tmpStr;
				startIndex = i + 1;
			}
		} else if (ch == '\\') {
			isInEsc = !isInEsc;
			if (!isInEsc) {
			}
		} else {
			isInEsc = 0; // false
		}
	}
	if (isInEsc) {
		result->length(0);
		return new IT_seq_str(result.in());
	}
	if (startIndex < len) {
		resultLen ++;
		result->length(resultLen);
		result[resultLen-1] = CORBA::string_dup(&str[startIndex]);
	} else if (ch == delimiter) {
		resultLen ++;
		result->length(resultLen);
		result[resultLen-1] = CORBA::string_dup("");
	}

	return new IT_seq_str(result.in());
}





static char *
removeEscChars(const char * str)
{
	int				i;
	char				ch;
	int				len;
	string				buf;
	CORBA::Boolean			isInEsc;

	len = strlen(str);
	isInEsc = 0; // false
	for (i = 0; i < len; i++) {
		ch = str[i];
		if (isInEsc) {
			buf += ch;
			isInEsc = 0; // false
		} else if (ch == '\\') {
			isInEsc = !isInEsc;
		} else {
			buf += ch;
		}
	}
	return CORBA::string_dup(buf.c_str());
}





static char *
getPathInNsFromInstructions(const char * instructions)
{
	const char *			start;
	const char *			end;
	char *				result;

	//--------
	// The path_in_ns is terminated by '\0' or a non-escaped whitespace.
	//--------
	start = instructions + strlen(ns_prefix);
	for (end = start; *end != '\0' && !isspace(*end); end++) {
		if (*end == '\\' && *(end+1) != '\0') {
			end++;
		}
	}

	//--------
	// Make a copy of the string from start to (end-1)
	// and null-terminate it
	// Notes:
	//	1. CORBA::string_alloc() allocates an extra byte for '\0'
	//	2. strncpy() null-terminates the copied string.
	//--------
	result = CORBA::string_alloc(end-start);
	strncpy(result, start, end-start);
	result[end-start] = '\0';
	return result;
}





static char *
getNsAddressFromInstructions(const char * instructions)
{
	const char *			start;
	const char *			ptr;
	char *				result;

	//--------
	// The path_in_ns is terminated by '\0' or a non-escaped whitespace.
	//--------
	start = instructions + strlen(ns_prefix);
	for (ptr = start; *ptr != '\0' && !isspace(*ptr); ptr++) {
		if (*ptr == '\\' && *(ptr+1) != '\0') {
			ptr++;
		}
	}

	//--------
	// Skip whitespace before an optional '@'
	//--------
	while (*ptr != '\0' && isspace(*ptr)) {
		ptr++;
	}

	if (*ptr == '\0') {
		return CORBA::string_dup("");
	}

	if (*ptr != '@') {
		string msg = string("Badly formatted import/export ")
			+ "instructions '" + instructions
			+ "': was expecting '@' after the "
			+ "path-in-naming-service";
		throw ImportExportException(msg);
	}

	//--------
	// Skip over '@' and optional whitespace before NS address
	//--------
	ptr++;
	while (*ptr != '\0' && isspace(*ptr)) {
		ptr++;
	}

	if (*ptr == '\0') {
		string msg = string("Badly formatted import/export ")
			+ "instructions '" + instructions
			+ "': was expecting a Naming Service address after '@'";
		throw ImportExportException(msg);
	}
	result = CORBA::string_dup(ptr);
	return result;
}





}; // namespace corbautil

