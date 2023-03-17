#include "headers/timer.h"
#include "headers/printer.h"
#include "headers/nameserver.h"
#include "headers/trainstop.h"

#define p (*pimpl)

class Timer::PImpl {
  public:
	Printer & prt;
	NameServer & nameServer;
	unsigned int timerDelay;

	PImpl( Printer & prt, NameServer & nameServer, unsigned int timerDelay ) :
		prt{ prt }, nameServer{ nameServer }, timerDelay{ timerDelay } {}
};// Timer::PImpl


Timer::Timer( Printer & prt, NameServer & nameServer, unsigned int timerDelay ) {
	pimpl = new PImpl { prt, nameServer, timerDelay };
} // Timer::Timer


Timer::~Timer() {
	delete pimpl;
} // Timer::~Timer


void Timer::main() {
	p.prt.print( Printer::Kind::Timer, 'S' );
	
	TrainStop ** trainStops = p.nameServer.getStopList();
	unsigned int numStops = p.nameServer.getNumStops();
	unsigned int tickCount = 0;
	
	// "It calls tick on each train stop after having delayed timerDelay 
	//  times by calling yield."
	for ( ;; ) {
		_Accept( ~Timer ) {
			break;
		} _Else {
			yield( p.timerDelay );
			p.prt.print( Printer::Kind::Timer, 't', tickCount );
			tickCount += 1;
			// Tick on each train
			for ( unsigned int i = 0; i < numStops; i += 1 ) {
				trainStops[i]->tick();
			} // for
		} // _Accept
	} // for

	p.prt.print( Printer::Kind::Timer, 'F' );
} // Timer::main

