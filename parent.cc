#include <uPRNG.h>
#include <algorithm>

#include "headers/parent.h"
#include "headers/bank.h"
#include "headers/printer.h"

#define p (*pimpl)


class Parent::PImpl {
  public:
	Printer & prt;
	Bank & bank;
	unsigned int numStudents, parentalDelay, maxTripCost;

	PImpl( Printer & prt, Bank & bank, unsigned int numStudents, unsigned int parentalDelay, unsigned int maxTripCost ) :
		prt{ prt }, bank{ bank }, numStudents{ numStudents }, parentalDelay{ parentalDelay }, maxTripCost{ maxTripCost } {}
};


Parent::Parent( Printer & prt, Bank & bank, unsigned int numStudents, unsigned int parentalDelay, unsigned int maxTripCost ) {
	pimpl = new PImpl( prt, bank, numStudents, parentalDelay, maxTripCost );
} // Parent::Parent


Parent::~Parent() {
	delete pimpl;
} // Parent::~Parent


void Parent::main() {
	p.prt.print( Printer::Kind::Parent, 'S' );
	
	for ( ;; ) {
		_Accept( ~Parent ) {
			break;
		} _Else {
			yield( p.parentalDelay );
			unsigned int gift = std::max((unsigned int)1, prng( 1, 3 ) * p.maxTripCost / 3 );
			unsigned int student = prng( p.numStudents );
			p.bank.deposit( student, gift );
			p.prt.print( Printer::Kind::Parent, 'D', student, gift );
		} // _Accept
	} // for
	p.prt.print( Printer::Kind::Parent, 'F' );
} // Parent::main
