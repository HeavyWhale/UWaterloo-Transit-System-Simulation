#include "headers/conductor.h"
#include "headers/train.h"
#include "headers/printer.h"

#define p (*pimpl)


class Conductor::PImpl {
  public:
	// Internal data
	Printer & prt;
	unsigned int id, delay;
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

	// "Its main loop consists of yielding the CPU conductor Delay times 
	//  and then calling the train’s scanPassengers method, which lets it 
	//  check all passengers on board to verify that they have a ticket."
	for ( ;; ) {
		_Accept( ~Conductor ) {
			break;
		} _Else {
			yield( p.delay );
			p.prt.print( Printer::Kind::Conductor, p.id, 'c' );
			p.train->scanPassengers();
		} // _Accept
	} // for

	p.prt.print( Printer::Kind::Conductor, p.id, 'F' );
} // Conductor::main
