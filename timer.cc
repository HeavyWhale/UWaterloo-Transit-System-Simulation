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


// TI1. A Timer’s function is to keep the simulation clock ticking 
//      by advancing the simulation in measured increments.
void Timer::main() {
	p.prt.print( Printer::Kind::Timer, 'S' );
	
	// NS4(a-i). The timer task ... obtain(s) the list of stops
	//           from the name server by calling getStopList.
	TrainStop ** trainStops = p.nameServer.getStopList();
	unsigned int numStops = p.nameServer.getNumStops();
	unsigned int tickCount = 0;
	
	for ( ;; ) {
		_Accept( ~Timer ) {
			break;
		} _Else {
			// TI2. It calls tick on each train stop after having delayed timerDelay times by calling yield.
			yield( p.timerDelay );
			p.prt.print( Printer::Kind::Timer, 't', tickCount );
			tickCount += 1;
			// TS15(i). The Timer calls tick to advance the system clock, 
			//          waking any trains blocked at this stop.	
			// TR6(iii). It visits each stop in turn at every “tick” of the Timer task.
			for ( unsigned int i = 0; i < numStops; i += 1 ) {
				trainStops[i]->tick();
			} // for
			// .(iii)6RT
			// .(i)51ST
			// .2IT
		} // _Accept
	} // for

	p.prt.print( Printer::Kind::Timer, 'F' );
} // Timer::main
// .1IT

