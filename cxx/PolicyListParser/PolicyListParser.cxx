//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	PolicyListParser.cxx
//
// Description: 
//----------------------------------------------------------------------





//--------
// #include's
//--------
#include "PolicyListParser.h"
#include "p_iostream.h"
#include "PoaUtility.h"
#include <assert.h>
#if P_ORBIX_VERSION >= 61
#include "orbix/leasing.hh"
#endif





//--------
// Singleton object
//--------
corbautil::ExtendablePolicyFactory corbautil::defaultPolicyFactory;





//--------
// Forward declaration of static functions
//--------
namespace corbautil {
	static CORBA::Boolean isPredefinedPolicyName(const char * name);
}





//----------------------------------------------------------------------
// Function:	Output streaming operator for corbautil::TokenKind
//----------------------------------------------------------------------

ostream& operator << (ostream & out, const corbautil::TokenKind & tk)
{
	switch (tk) {
	case corbautil::t_ident:
		out	<< "t_ident";
		break;
	case corbautil::t_equals:
		out	<< "t_equals";
		break;
	case corbautil::t_open_p:
		out	<< "t_open_p";
		break;
	case corbautil::t_close_p:
		out	<< "t_close_p";
		break;
	case corbautil::t_comma:
		out	<< "t_comma";
		break;
	case corbautil::t_EOS:
		out	<< "t_EOS";
		break;
	default:
		assert(0); // Bug!
	}
	return out;
}





//----------------------------------------------------------------------
// Function:	Output streaming operator for corbautil::Token
//----------------------------------------------------------------------

ostream& operator << (ostream & out, const corbautil::Token & t)
{
	out	<< "token(kind="
		<< t.kind()
		<< "; spelling='"
		<< t.spelling()
		<< "')";
	return out;
}





namespace corbautil
{





//======================================================================
// LexicalAnalyser
//======================================================================





//----------------------------------------------------------------------
// Function:	Constructor
//
// Description:	
//----------------------------------------------------------------------

LexicalAnalyser::LexicalAnalyser(const char * str)
{
	m_str            = CORBA::string_dup(str);
	m_len            = strlen(m_str);
	m_i              = 0;
	m_in_parenthesis = 0;
}





//----------------------------------------------------------------------
// Function:	Destructor
//
// Description:	
//----------------------------------------------------------------------

LexicalAnalyser::~LexicalAnalyser()
{
	// nothing to do
}





//----------------------------------------------------------------------
// Function:	reset()
//
// Description:	
//----------------------------------------------------------------------

void
LexicalAnalyser::reset(const char * str)
{
	m_str            = CORBA::string_dup(str);
	m_len            = strlen(m_str);
	m_i              = 0;
	m_in_parenthesis = 0;
}





//----------------------------------------------------------------------
// Function:	nextToken()
//
// Description:	
//----------------------------------------------------------------------

void
LexicalAnalyser::nextToken(Token & tok)
{
	CORBA::ULong	start;
	char *		identSpelling;

	//--------
	// Skip over whitespace. Note that a plus sign is always consisdered
	// to be whitespace and a comma is considered to be whitespace if
	// it appears OUTSIDE of a parenthesis-enclosed parameter list.
	//--------
	start = m_i;
	while (start < m_len &&
	       (isspace(m_str[start])
	        || m_str[start] == '+'
	        || (m_str[start] == ',' && !m_in_parenthesis)
	       )
	      )
	       {
		start++;
	}

	//--------
	// Are we at the end-of-string (EOS)?
	//--------
	if (start == m_len) {
		m_i = start;
		tok.kind(t_EOS);
		tok.spelling("<end-of-policy-list>");
		return;
	}

	//--------
	// Handle identifiers (which actually include numbers too)
	//--------
	if (isValidIdentChar(m_str[start])) {
		m_i = start;
		while (m_i < m_len && isValidIdentChar(m_str[m_i])) {
			m_i ++;
		}
		identSpelling = new char[m_i - start + 1];
		strncpy(identSpelling, &m_str[start], m_i-start);
		identSpelling[m_i-start] = '\0';
		tok.reset(t_ident, identSpelling);
		delete [] identSpelling;
		return;
	}

	//--------
	// Handle string literal values
	//--------
	if (m_str[start] == '\'') {
		m_i = start;
		while (m_i < m_len && m_str[m_i] != '\'') {
			m_i ++;
		}
		if (m_str[m_i] != '\'') {
			throw ParserException(
				"Unterminated string literal in policy list");
		}
		start ++; // skip after opening '\''
		identSpelling = new char[m_i - start + 1];
		strncpy(identSpelling, &m_str[start], m_i-start);
		identSpelling[m_i-start] = '\0';
		tok.reset(t_ident, identSpelling);
		delete [] identSpelling;
		m_i ++; // skip over closing '\''
		return;
	}

	//--------
	// Handle other, miscellaneous characters
	//--------
	switch (m_str[start]) {
	case '(':
		if (m_in_parenthesis) {
			throw ParserException(
				"Unexpected '(' inside a parameter list");
		}
		m_in_parenthesis = 1;
		tok.reset(corbautil::t_open_p, "(");
		m_i = start + 1;
		break;
	case ')':
		if (!m_in_parenthesis) {
			throw ParserException(
				"Unexpected ')' outside a parameter list");
		}
		m_in_parenthesis = 0;
		tok.reset(corbautil::t_close_p, ")");
		m_i = start + 1;
		break;
	case ',':
		assert(m_in_parenthesis);
		tok.reset(corbautil::t_comma, "'");
		m_i = start + 1;
		break;
	case '=':
		tok.reset(corbautil::t_equals, "=");
		m_i = start + 1;
		break;
	default:
		char	msg[64]; // Big enough
		if (isprint(m_str[start])) {
			sprintf(msg,
				"Unexpected character '%c' in policy list",
				m_str[start]);
		} else {
			sprintf(msg,
				"Unexpected character (%d) in policy list",
				(unsigned int)m_str[start]);
		}
		throw ParserException(msg);
		break;
	}
}





//----------------------------------------------------------------------
// Function:	isValidIdentChar
//
// Description:	
//----------------------------------------------------------------------

CORBA::Boolean
LexicalAnalyser::isValidIdentChar(char ch)
{
	if (isalnum(ch)) {
		return 1;
	}
	switch (ch)
	{
	case '_':
	case ':':
	case '.':
		return 1;
	}
	return 0;
}





//======================================================================
// PolicyFactory
//======================================================================





CORBA::Boolean
PolicyFactory::stringToBoolean(const char * str) throw(ParserException)
{
	CORBA::Boolean		result;

	if (strcmp("true", str) == 0) {
		result = 1;
	} else if (strcmp("false", str) == 0) {
		return 0;
	} else {
		strstream	buf;
		buf	<< "illegal boolean value ('"
			<< str
			<< "') used as a parameter in policy list"
			<< ends;
		throw ParserException(buf);
	}
	return result;
}





CORBA::Long
PolicyFactory::stringToLong(const char * str) throw(ParserException)
{
	int			i;
	int			val;
	char			dummy;

	i = sscanf(str, "%d%c", &val, &dummy);
	if (i != 1) {
		strstream	buf;
		buf	<< "illegal integer value ('"
			<< str
			<< "') used as a parameter in policy list"
			<< ends;
		throw ParserException(buf);
	}
	return val;
}





CORBA::Float  
PolicyFactory::stringToFloat(const char * str) throw(ParserException)
{
	int			i;
	float			val;
	char			dummy;

	i = sscanf(str, "%f%c", &val, &dummy);
	if (i != 1) {
		strstream	buf;
		buf	<< "illegal floating-point value ('"
			<< str
			<< "') used as a parameter in policy list"
			<< ends;
		throw ParserException(buf);
	}
	return val;
}





PortableServer::POA_ptr
PolicyFactory::rootPoa(CORBA::ORB_ptr orb) throw(ParserException)
{
	CORBA::Object_var		tmpObj;
	PortableServer::POA_var		rootPoa;

	try {
		tmpObj  = orb->resolve_initial_references("RootPOA");
		rootPoa = POA::_narrow(tmpObj);
		assert(!CORBA::is_nil(rootPoa));
	} catch (const CORBA::Exception & ex) {
		strstream	buf;
		buf	<< "resolve_initial_references(\"RootPOA\") failed: "
			<< ex
			<< ends;
		throw ParserException(buf);
	}
	assert(!CORBA::is_nil(rootPoa));
	return rootPoa._retn();
}





//======================================================================
// ExtendablePolicyFactory
//======================================================================





ExtendablePolicyFactory::ExtendablePolicyFactory()
{
	// nothing to do
}





ExtendablePolicyFactory::~ExtendablePolicyFactory()
{
	// nothing to do
}





void
ExtendablePolicyFactory::registerPolicyFactory(
	const char *			policyName,
	corbautil::PolicyFactory *	factory)
{
	PolicyFactoryList *		ptr;

	//--------
	// Check that the policyName does not conflict with any
	// predefined policies
	//--------
	if (isPredefinedPolicyName(policyName)) {
		strstream	buf;
		buf	<< "PoaUtility::registerPolicyFactory() "
			<< "failed: attempted registration of '"
			<< policyName
			<< "' conflicts with a predefined policy " 
			<< "of the same name"
			<< ends;
		throw ParserException(buf);
	}

	//--------
	// Check that a similarly-named factory is not already registered
	//--------
	for (ptr = m_head.m_next; ptr != 0; ptr = ptr->m_next) {
		if (strcmp(policyName, ptr->m_name.in()) == 0) {
			strstream	buf;
			buf	<< "PoaUtility::registerPolicyFactory() "
				<< "failed: attempted re-registration of '"
				<< policyName
				<< "' policy" 
				<< ends;
			throw ParserException(buf);
		}
	}

	//--------
	// Insert the factory at the start of the list
	//--------
	ptr = new PolicyFactoryList();
	ptr->m_factory = factory;
	ptr->m_name = CORBA::string_dup(policyName);
	ptr->m_next = m_head.m_next;
	m_head.m_next = ptr;
}





CORBA::Policy_ptr
ExtendablePolicyFactory::create(
	CORBA::ORB_ptr			orb,
	const char *			name,
	NameValue *			nvArray,
	int				nvArraySize)
{
	CORBA::Policy_var		result;
	PolicyFactoryList *		ptr;
	CORBA::String_var		parameterlessName;
	strstream			buf;


	if (isPredefinedPolicyName(name) && nvArraySize != 0) {
		buf << "policy '" << name << "' does not take any parameters"
		    << ends;
		throw ParserException(buf);

	}

	if (strcmp(name, "orb_ctrl_model") == 0) {
		result = rootPoa(orb)->create_thread_policy(ORB_CTRL_MODEL);
	} else if (strcmp(name, "single_thread_model") == 0) {
		result = rootPoa(orb)->create_thread_policy(
							SINGLE_THREAD_MODEL);
	} else if (strcmp(name, "transient") == 0) {
		result = rootPoa(orb)->create_lifespan_policy(TRANSIENT);
	} else if (strcmp(name, "persistent") == 0) {
		result = rootPoa(orb)->create_lifespan_policy(PERSISTENT);
	} else if (strcmp(name, "unique_id") == 0) {
		result = rootPoa(orb)->create_id_uniqueness_policy(UNIQUE_ID);
	} else if (strcmp(name, "multiple_id") == 0) {
		result = rootPoa(orb)->create_id_uniqueness_policy(MULTIPLE_ID);
	} else if (strcmp(name, "user_id") == 0) {
		result = rootPoa(orb)->create_id_assignment_policy(USER_ID);
	} else if (strcmp(name, "system_id") == 0) {
		result = rootPoa(orb)->create_id_assignment_policy(SYSTEM_ID);
	} else if (strcmp(name, "implicit_activation") == 0) {
		result = rootPoa(orb)->create_implicit_activation_policy(
					IMPLICIT_ACTIVATION);
	} else if (strcmp(name, "no_implicit_activation") == 0) {
		result = rootPoa(orb)->create_implicit_activation_policy(
					NO_IMPLICIT_ACTIVATION);
	} else if (strcmp(name, "retain") == 0) {
		result = rootPoa(orb)->create_servant_retention_policy(RETAIN);
	} else if (strcmp(name, "non_retain") == 0) {
		result = rootPoa(orb)->create_servant_retention_policy(
								NON_RETAIN);
	} else if (strcmp(name, "use_active_object_map_only") == 0) {
		result = rootPoa(orb)->create_request_processing_policy(
					USE_ACTIVE_OBJECT_MAP_ONLY);
	} else if (strcmp(name, "use_default_servant") == 0) {
		result = rootPoa(orb)->create_request_processing_policy(
					USE_DEFAULT_SERVANT);
	} else if (strcmp(name, "use_servant_manager") == 0) {
		result = rootPoa(orb)->create_request_processing_policy(
					USE_SERVANT_MANAGER);
#if defined(P_USE_ORBIX)
	} else if (strcmp(name, "orbix.deliver") == 0) {
		CORBA::Any	a;
		a <<= IT_PortableServer::DELIVER;
		result = orb->create_policy(
			IT_PortableServer::OBJECT_DEACTIVATION_POLICY_ID, a);
	} else if (strcmp(name, "orbix.discard") == 0) {
		CORBA::Any	a;
		a <<= IT_PortableServer::DISCARD;
		result = orb->create_policy(
			IT_PortableServer::OBJECT_DEACTIVATION_POLICY_ID, a);
	} else if (strcmp(name, "orbix.hold") == 0) {
		CORBA::Any	a;
		a <<= IT_PortableServer::HOLD;
		result = orb->create_policy(
			IT_PortableServer::OBJECT_DEACTIVATION_POLICY_ID, a);
#if P_ORBIX_VERSION >= 61
	} else if (strcmp(name, "orbix.no_lease") == 0) {
		CORBA::Any	a;
		a <<= CORBA::Any::from_boolean(0);
		result = orb->create_policy(IT_Leasing::LEASING_POLICY_ID, a);
#endif
#endif
	} else {
		for (ptr = m_head.m_next; ptr != 0; ptr = ptr->m_next) {
			if (strcmp(ptr->m_name.in(), name) == 0) {
				try {
					return ptr->m_factory->create(
							orb, name,
							nvArray, nvArraySize);
				} catch(const ParserException &) {
					throw;
				} catch(const CORBA::Exception & ex) {
					buf	<< ex
						<< ends;
					throw ParserException(buf);
				}
			}
		}

		//--------
		// Illegal policy name. Throw back a descriptive exception.
		//--------
		buf << "illegal policy name" << ends;
		throw ParserException(buf);
	}
	return result._retn();
}





//======================================================================
// PolicyListParser
//======================================================================





PolicyListParser::PolicyListParser(
	CORBA::ORB_ptr			orb,
	PolicyFactory *			factory)
		: m_lex("")
{
	m_orb             = orb;
	m_factory         = factory;
	m_policy_list_str = "";
	m_lex.nextToken(m_token);

	//--------
	// Pre-allocate some space for the sequence of policies
	//--------
	m_seq.length(7);
	m_seq.length(0);
}





CORBA::PolicyList *
PolicyListParser::parsePolicyList(const char * policy_list_str)
{
	m_policy_list_str = policy_list_str;
	m_lex.reset(policy_list_str);
	m_lex.nextToken(m_token);
	while (m_token.kind() == t_ident) {
		parsePolicy();
	}
	accept(t_EOS, "unexpected end of policy list");
	return new CORBA::PolicyList(m_seq);
}





void
PolicyListParser::parsePolicy()
{
	CORBA::String_var		policyName;
	CORBA::StringSeq		paramNameSeq(20);
	CORBA::StringSeq		paramValueSeq(20);
	CORBA::ULong			paramCount;

	//--------
	// Record the name of the policy
	//--------
	policyName = CORBA::string_dup(m_token.spelling());
	m_lex.nextToken(m_token);

	//--------
	// If there is no parameter list for the policy then process
	// it immediately and return.
	//--------
	if (m_token.kind() != t_open_p) {
		processPolicy(policyName.in(), paramNameSeq, paramValueSeq);
		return;
	}

	//--------
	// There is a parameter list. Parse it.
	//--------
	assert(m_token.kind() == t_open_p);
	m_lex.nextToken(m_token); // consume '('
	paramCount = 0;

	if (m_token.kind() != t_close_p) {
		parsePolicyParam(paramNameSeq, paramValueSeq, paramCount);
		while (m_token.kind() == t_comma) {
			m_lex.nextToken(m_token); // consume ','
			parsePolicyParam(paramNameSeq, paramValueSeq,
						paramCount);
		}
	}
	accept(t_close_p, "Expecting \",\" or \")\"");

	//--------
	// Process the policy.
	//--------
	processPolicy(policyName.in(), paramNameSeq, paramValueSeq);
}





void
PolicyListParser::parsePolicyParam(
	CORBA::StringSeq &	paramNameSeq,
	CORBA::StringSeq &	paramValueSeq,
	CORBA::ULong &		paramCount)
{
	CORBA::String_var		ident1;
	CORBA::String_var		ident2;

	ident1 = CORBA::string_dup(m_token.spelling());
	accept(t_ident, "Expecting a parameter name, value or \")\"");
	if (m_token.kind() == t_equals) {
		m_lex.nextToken(m_token); // consume '='
		ident2 = CORBA::string_dup(m_token.spelling());
		accept(t_ident, "Expecting a parameter value");
		paramCount++;
		paramNameSeq.length(paramCount);
		paramValueSeq.length(paramCount);
		paramNameSeq[paramCount-1]  = CORBA::string_dup(ident1);
		paramValueSeq[paramCount-1] = CORBA::string_dup(ident2);
	} else {
		paramCount++;
		paramNameSeq.length(paramCount);
		paramValueSeq.length(paramCount);
		paramNameSeq[paramCount-1]  = CORBA::string_dup("");
		paramValueSeq[paramCount-1] = CORBA::string_dup(ident1);
	}
}





void
PolicyListParser::accept(TokenKind expectedKind, const char * errMsg)
{
	if (m_token.kind() == expectedKind) {
		m_lex.nextToken(m_token);
	} else {
		strstream	buf;

		buf	<< "error parsing policy list '"
			<< m_policy_list_str
			<< "' near '"
			<< m_token.spelling()
			<< "': "
			<< errMsg
			<< endl;
		throw corbautil::ParserException(buf);
	}
}





void
PolicyListParser::processPolicy(
	const char *			name,
	const CORBA::StringSeq &	paramNameSeq,
	const CORBA::StringSeq &	paramValueSeq)
{
	strstream			buf;
	CORBA::ULong			len;
	CORBA::ULong			i;
	NameValue *			nvArray;
	int				nvArraySize;

	assert(paramNameSeq.length() == paramValueSeq.length());
	nvArraySize = paramNameSeq.length();
	if (nvArraySize == 0) {
		nvArray = 0;
	} else {
		nvArray = new NameValue[nvArraySize];
		for (i = 0; i < nvArraySize; i++) {
			nvArray[i].name
				= CORBA::string_dup(paramNameSeq[i]);
			nvArray[i].value
				= CORBA::string_dup(paramValueSeq[i]);
		}
	}

	len = m_seq.length();
	m_seq.length(len+1);
	try {
		m_seq[len] = m_factory->create(
					m_orb, name, nvArray, nvArraySize);
	} catch(const corbautil::ParserException & ex) {
		delete [] nvArray;
		buf	<< "error creating policy '"
			<< name
			<< "' in '"
			<< m_policy_list_str
			<< "': "
			<< ex
			<< endl;
		throw corbautil::ParserException(buf);
	} catch(const CORBA::Exception & ex) {
		delete [] nvArray;
		buf	<< "error creating policy '"
			<< name
			<< "' in '"
			<< m_policy_list_str
			<< "': "
			<< ex
			<< endl;
		throw corbautil::ParserException(buf);
	}
}





//======================================================================
// Static utility functions.
//======================================================================





static const char * predefined_policy_names[] = {
#if defined(P_USE_ORBIX)
	"orbix.deliver",
	"orbix.discard",
	"orbix.hold",
#if P_ORBIX_VERSION >= 61
	"orbix.no_lease",
#endif
#endif
	"orb_ctrl_model",
	"single_thread_model",
	"transient",
	"persistent",
	"unique_id",
	"multiple_id",
	"user_id",
	"system_id",
	"implicit_activation",
	"no_implicit_activation",
	"retain",
	"non_retain",
	"use_active_object_map_only",
	"use_default_servant",
	"use_servant_manager"
};





static CORBA::Boolean
isPredefinedPolicyName(const char * name)
{
	int			i;

	for (i = 0; i < sizeof(predefined_policy_names)/sizeof(char*); i++) {
		if (strcmp(name, predefined_policy_names[i]) == 0) {
			return 1;
		}
	}
	return 0;
}





}; // namespace corbautil
