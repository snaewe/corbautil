#include "PolicyListParser.h"
#include "p_iostream.h"



int
main(int argc, char ** argv)
{
	CORBA::ORB_var			orb;
	CORBA::PolicyList_var		seq;
	corbautil::PolicyListParser *	parser;
	int				exit_code;
	
	exit_code = 0;
	try {
		orb = CORBA::ORB_init(argc, argv);
		if (argc != 2) {
			cerr	<< "usage: " << argv[0]
				<< " policy-list" << endl;
			throw -1;
		}
		parser = new corbautil::PolicyListParser(orb);
		seq = parser->parsePolicyList(argv[1]);
		cout << "Parsed " << seq->length() << " policies" << endl;
	} catch(const CORBA::Exception & ex) {
		cout	<< "ORB_init() failed: " << ex << endl;
		exit_code = 1;
	} catch(const corbautil::ParserException & ex) {
		cout	<< ex << endl;
		exit_code = 1;
	} catch(int) {
		exit_code = 1;
	}

	try {
		if (!CORBA::is_nil(orb)) {
			orb->destroy();
		}
	} catch(CORBA::Exception & ex) {
		exit_code = 1;
		cout	<< "orb->destroy() failed: " << ex << endl;
	}
	delete parser;

	return exit_code;
}
