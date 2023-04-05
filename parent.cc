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
		// PA2(a). The parent must check for a call to its destructor to know when to terminate. ...
		_Accept( ~Parent ) {
			break;
		
		// PA1. The Parent task periodically gives a random amount of money 
		//      (one-third, two-thirds, or the full maximum cost of a trip) 
		//      i.e. std::max(1, stopCost × ⌊numStops / 2⌋ × [1,3] / 3) 
		//      to a random student.
		// PA2(b). ... Since it must not block on this call, it is necessary 
		//         to use a terminating _Else on the accept statement. Hence, 
		//         the parent is busy waiting for the call to its destructor
		} _Else {
			yield( p.parentalDelay );
			unsigned int gift = std::max(1U, prng( 1, 3 ) * p.maxTripCost / 3 );
			unsigned int student = prng( p.numStudents );
			// BA3(a). The parent calls deposit to endow gifts to a specific student.
			p.bank.deposit( student, gift );
			p.prt.print( Printer::Kind::Parent, 'D', student, gift );
		} // _Accept
		// .(b)2AP
		// .1AP
	} // for
	p.prt.print( Printer::Kind::Parent, 'F' );
} // Parent::main
