//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	import_export.h
//
// Description: Utility functions for importing and exporting object
//		references.
//
// Format of export instructions
// -----------------------------
//
//	"name_service#<path>"              Example: "name_service#foo/bar"
//	"file#<path/to/file>"              Example: "file#foo.ior"
//	"exec#<cmd with IOR placeholder>"  Example: "exec#echo IOR >foo.ior"
//	"corbaloc_server#<name>"           Example: "corbaloc_server#foo"
//
// Format of import instructions
// -----------------------------
//
//	"name_service#<path>"              Example: "name_service#foo/bar"
//	"file#<path/to/file>"              Example: "file#foo.ior"
//	"exec#<cmd>"                       Example: "exec#cat foo.ior"
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
// } catch (corbautil::ImportExportException & ex) {
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
// } catch (corbautil::ImportExportException & ex) {
//	cerr << ex << endl;
//	orb->destroy();
//	exit(1);
// }
// ... narrow "obj" to the desired type
//----------------------------------------------------------------------

#ifndef IMPORT_EXPORT_H_
#define IMPORT_EXPORT_H_



//--------
// #include's
//--------
#include "p_orb.h"
#include "p_iostream.h"
#include "p_strstream.h"
#include <string>




namespace corbautil
{

	class ImportExportException {
	public:
		ImportExportException()
		{
			msg = CORBA::string_dup("");
		}

		ImportExportException(std::string str)
		{
			msg = CORBA::string_dup(str.c_str());
		}

		ImportExportException(strstream & buf)
		{
			msg = CORBA::string_dup(buf.str());
			buf.rdbuf()->freeze(0);
		}

		CORBA::String_var	msg;
	};

	void
	exportObjRef(
		CORBA::ORB_ptr			orb,
		CORBA::Object_ptr		obj,
		const char *			instructions)
			throw(ImportExportException);

	CORBA::Object_ptr
	importObjRef(
		CORBA::ORB_ptr		orb,
		const char *		instructions)
			throw(ImportExportException);

}; // namespace corbautil

inline ostream& operator << (
	ostream &					out,
	const corbautil::ImportExportException &	ex)
{
	out	<< ex.msg.in();
	return out;
}



#endif
