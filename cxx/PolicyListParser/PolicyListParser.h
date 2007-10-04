//----------------------------------------------------------------------
// Copyright (c) 2002-2005 IONA Technologies. All rights reserved.
// This software is provided "as is".
//
// File:	PolicyListParser.h
//
// Description: 
//----------------------------------------------------------------------

#ifndef POLICY_LIST_PARSER_H_
#define POLICY_LIST_PARSER_H_





//--------
// #include's
//--------
#include "p_orb.h"
#include "p_poa.h"
#include "p_strstream.h"
#include "p_iostream.h"





namespace corbautil
{
	enum TokenKind {t_ident, t_equals, t_open_p, t_close_p, t_comma, t_EOS};

	//--------------------------------------------------------------
	// Class:	ParserException
	//
	// Description:	Exception that might be thrown by the parser or
	//		lexical analyser.
	//--------------------------------------------------------------
	class ParserException {
	public:
		ParserException(const char * str)
		{
			msg = CORBA::string_dup(str);
		}
		ParserException(strstream & buf)
		{
			msg = CORBA::string_dup(buf.str());
			buf.rdbuf()->freeze(0);
		}
		ParserException(const ParserException & other)
		{
			msg = CORBA::string_dup(other.msg.in());
		}

		CORBA::String_var	msg;
	private:
		//--------
		// The following are not implemented
		//--------
		ParserException();
		ParserException& operator=(const ParserException &);
	};





	//--------------------------------------------------------------
	// Class:	Token
	//
	// Descritpion:	Tokens created by the lexical analyser.
	//--------------------------------------------------------------
	class Token {
	public:
		//--------
		// Constructors, destructor and assignment operator
		//--------
		inline Token(
			TokenKind	kind = t_EOS,
			const char *	spelling = "");
		inline Token(const Token & other);
		inline ~Token();

		//--------
		// Assignment operator
		//--------
		inline Token & operator=(const Token & other);

		//--------
		// Accessor and modifier operations
		//--------
		inline void reset(TokenKind kind, const char * spelling);

		inline TokenKind     kind() const;
		inline void          kind(TokenKind spelling);

		inline const char *  spelling() const;
		inline void          spelling(const char * spelling);

	private:
		//--------
		// Instance variables
		//--------
		TokenKind		m_kind;
		CORBA::String_var	m_spelling;
	};





	//--------------------------------------------------------------
	// Class:	LexicalAnalyser
	//
	// Descritpion:	Lexical analyser used by PolicyListParser.
	//--------------------------------------------------------------
	class LexicalAnalyser
	{
	public:
		LexicalAnalyser(const char * str = "");
		~LexicalAnalyser();

		void reset(const char * str);
		void nextToken(Token & tok);
	private:
		//--------
		// Helper operations
		//--------
		CORBA::Boolean isValidIdentChar(char ch);

		//--------
		// Instance variables
		//--------
		CORBA::String_var	m_str;
		CORBA::ULong		m_i;
		CORBA::ULong		m_len;
		CORBA::Boolean		m_in_parenthesis;

		//--------
		// The following are not implemented
		//--------
		LexicalAnalyser(const LexicalAnalyser &);
		LexicalAnalyser & operator = (const LexicalAnalyser &);
	};





	//--------------------------------------------------------------
	// Class:	NameValue
	//
	// Descritpion:	A name-value pair.
	//--------------------------------------------------------------
	struct NameValue {
		CORBA::String_var	name;
		CORBA::String_var	value;
	};





	//--------------------------------------------------------------
	// Class:	PolicyFactory
	//
	// Descritpion:	A class with a create() operation for creating
	//		Policy objects. This class also has some utility
	//		operations that help convert stringified values
	//		to common types.
	//--------------------------------------------------------------
	class PolicyFactory {
	public:
		PolicyFactory() {}
		virtual ~PolicyFactory() {}

		//--------
		// create()
		//--------
		virtual CORBA::Policy_ptr create(
				CORBA::ORB_ptr	orb,
				const char *	policyName,
				NameValue *	nvArray,
				int		nvArraySize) = 0;

		//--------
		// Utility string-to-<type> conversion operations.
		//--------
		CORBA::Boolean	stringToBoolean(const char * str)
					throw(ParserException);
		CORBA::Long	stringToLong(const char * str)
					throw(ParserException);
		CORBA::Float	stringToFloat(const char * str)
					throw(ParserException);

		//--------
		// Utility function. Obtain root POA from the ORB
		//--------
		PortableServer::POA_ptr rootPoa(CORBA::ORB_ptr orb)
					throw(ParserException);
	};





	//--------------------------------------------------------------
	// Class:	PolicyFactoryList
	//
	// Descritpion:	Singly-linked list of
	//		(policyName, PolicyFactory) tuples.
	//--------------------------------------------------------------
	class PolicyFactoryList {
	public:
		PolicyFactoryList()
		{
			m_factory = 0;
			m_next    = 0;
		}
		~PolicyFactoryList()
		{
			delete m_next;
			delete m_factory;
		}
		PolicyFactory *		m_factory;
		CORBA::String_var	m_name;
		PolicyFactoryList *	m_next;
	};





	//--------------------------------------------------------------
	// Class:	ExtendablePolicyFactory
	//
	// Descritpion:	
	//--------------------------------------------------------------
	class ExtendablePolicyFactory : public PolicyFactory
	{
	public:
		ExtendablePolicyFactory();
		virtual ~ExtendablePolicyFactory();

		//--------
		// This class re-implements the create() operation.
		//--------
		virtual CORBA::Policy_ptr create(
				CORBA::ORB_ptr			orb,
				const char *			policyName,
				NameValue *			nvArray,
				int				nvArraySize);

		//--------
		// Register a factory with us to extend our functionality.
		//--------
		void registerPolicyFactory(
				const char *			policyName,
				corbautil::PolicyFactory *	factory);

	private:
		//--------
		// Instance variables.
		//--------
		PolicyFactoryList	m_head;

		//--------
		// The following are not implemented.
		//--------
		ExtendablePolicyFactory(const ExtendablePolicyFactory &);
		ExtendablePolicyFactory & operator=(
					const ExtendablePolicyFactory &);
	};





	//--------------------------------------------------------------
	// Singleton factory object.
	//--------------------------------------------------------------
	extern ExtendablePolicyFactory defaultPolicyFactory;





	//--------------------------------------------------------------
	// Class:	PolicyListParser
	//
	// Descritpion:	Parser for a stringified list of policy values.
	//--------------------------------------------------------------
	class PolicyListParser
	{
	public:
		//--------
		// Constructor and destructor
		//--------
		PolicyListParser(
			CORBA::ORB_ptr	orb,
			PolicyFactory *	factory = & defaultPolicyFactory);
		~PolicyListParser() { }

		CORBA::PolicyList *	parsePolicyList(
						const char * policy_list_str);

	private:
		//--------
		// Helper operations.
		//--------
		void parsePolicy();

		void parsePolicyParam(
			CORBA::StringSeq &	paramNameSeq,
			CORBA::StringSeq &	paramValueSeq,
			CORBA::ULong &		paramCount);

		void processPolicy(
			const char *			policyName,
			const CORBA::StringSeq &	paramNameSeq,
			const CORBA::StringSeq &	paramValueSeq);

		void accept(TokenKind expectedKind, const char * errMsg);
	private:
		//--------
		// Instance variables
		//--------
		LexicalAnalyser		m_lex;
		Token			m_token;
		const char *		m_policy_list_str;
		CORBA::PolicyList	m_seq;
		CORBA::ORB_ptr		m_orb;
		PolicyFactory *		m_factory;

		//--------
		// The following are not implemented
		//--------
		PolicyListParser(const PolicyListParser &);
		PolicyListParser & operator = (const PolicyListParser &);
	};





	//--------------------------------------------------------------
	// Start of inline implementation for the Token class
	//--------------------------------------------------------------
	inline Token::Token(TokenKind kind, const char * spelling)
	{
		m_kind = kind;
		m_spelling = CORBA::string_dup(spelling);
	}
	inline Token::Token(const Token & other)
	{
		m_kind = other.m_kind;
		m_spelling = CORBA::string_dup(other.m_spelling);
	}
	inline Token::~Token() { }

	inline Token & Token::operator=(const Token & other)
	{
		m_kind = other.m_kind;
		m_spelling = CORBA::string_dup(other.m_spelling);
		return *this;
	}

	inline void Token::reset(TokenKind kind, const char * spelling)
	{
		m_kind = kind;
		m_spelling = CORBA::string_dup(spelling);
	}
	inline TokenKind Token::kind() const         { return m_kind; }
	inline void      Token::kind(TokenKind kind) { m_kind = kind; }

	inline const char * Token::spelling() const  { return m_spelling.in(); }
	inline void         Token::spelling(const char * spelling)
	                    { m_spelling = CORBA::string_dup(spelling); }
	//--------------------------------------------------------------
	// End of inline implementation for the Token class
	//--------------------------------------------------------------





}; // namespace corbautil

//--------
// Streaming operators
//--------
ostream& operator << (ostream & out, const corbautil::TokenKind & tk);
ostream& operator << (ostream & out, const corbautil::Token & t);
inline ostream& operator << (
	ostream &				out,
	const corbautil::ParserException &	ex)
{
	out	<< ex.msg.in();
	return out;
}


#endif /* POLICY_LIST_PARSER_H_ */
