#include "headers/conductor.h"
#include "headers/train.h"
#include "headers/printer.h"

#define p (*pimpl)


class Conductor::PImpl {
  public:
	// Internal data
	Printer & prt;
	unsigned int id, delay;
	// TR10(a). Each train has a Conductor, ...
	// CO1(ii). There is one conductor per train, with the same id number as the train it patrols.
	Train * train;

	PImpl( Printer & prt, unsigned int id, unsigned int delay, Train * train ) :
		prt{ prt }, id{ id }, delay{ delay }, train{ train } {}
};// Conductor::PImpl


Conductor::Conductor( Printer & prt, unsigned int id, Train * train, unsigned int delay ) {
	pimpl = new PImpl { prt, id, delay, train };
} // Conductor::Conductor


Conductor::~Conductor() {
	delete pimpl;
} // Conductor::~Conductor


void Conductor::main() {
	p.prt.print(Printer::Kind::Conductor, p.id, 'S');

	for ( ;; ) {
		_Accept( ~Conductor ) {
			break;
		
		// TR10(b). ... responsible for checking periodically ...
		} _Else {
			// CO2(a). Its main loop consists of yielding the CPU conductorDelay times ...
			yield( p.delay );
			p.prt.print( Printer::Kind::Conductor, p.id, 'c' );
			// S18(b-i). ... or after being caught by a conductor and ejected from the train.
			// TS4(c-i). ... which is used as a proof-of-purchase to the  
			//           conductor that the student has paid their fare.
			// CO2(b). ... and then calling the train’s scanPassengers method, ...
			// WC7(i). A conductor calls paidForTicket to see the POP.
			p.train->scanPassengers();
		} // _Accept
		// .(b)01RT
	} // for

	p.prt.print( Printer::Kind::Conductor, p.id, 'F' );
} // Conductor::main
