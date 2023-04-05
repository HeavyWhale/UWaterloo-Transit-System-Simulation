#include "headers/train.h"
#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/train.h"
#include "headers/trainstop.h"
#include "headers/nameserver.h"

#include <cmath>
#include <iostream>

using namespace std;

#define p (*pimpl)

struct Seat {
	uCondition condSeat;
	bool isOccupied = false, isCaught = false;
	WATCard * card = nullptr;
	uBaseTask * student;
	TrainStop ** destTrainStop;
}; // Seat


class Train::PImpl {
  public:
	// Internal data
	Printer & prt;
	NameServer & nameServer;
	unsigned int id, maxNumStudents, numStops;

	// Communication vars
	unsigned int curStop, numStudents;
	Direction direction;
	struct Seat * seats;

	unsigned int sid;

	PImpl( Printer &prt, NameServer &nameServer, unsigned int id, 
		unsigned int maxNumStudents, unsigned int numStops) : 
		prt{ prt }, nameServer{ nameServer }, id{ id }, 
		maxNumStudents{ maxNumStudents }, numStops{ numStops }
	{
		// TR1(a). A Train’s function is to first determine its direction of travel, ...
		if ( id == 0 ) {
			// TR1(b). ... where train 0 travels in a clockwise direction starting at stop 0, ...
			direction = Direction::Clockwise;
			curStop = 0;
			// .(b)1RT
		} else {
			// TR1(c). ... while train 1 travels in a counter-clockwise direction starting at stop ⌈numStops/2⌉.
			direction = Direction::CounterClockwise;
			curStop = (numStops + 1) / 2;
			// .(c)1RT
		} // if
		// .(a)1RT
		seats = new Seat[ maxNumStudents ];
		numStudents = 0;
	} // Train::PImpl::PImpl

	~PImpl() {
		delete[] seats;
	} // Train::PImpl::~PImpl
}; // Trian::PImpl


Train::Train( Printer & prt, NameServer & nameServer, unsigned int id, 
	unsigned int maxNumStudents, unsigned int numStops ) 
{
	pimpl = new PImpl { prt, nameServer, id, maxNumStudents, numStops };
}


Train::~Train() {
	delete pimpl;
} // Train::~Train


_Nomutex unsigned int Train::getId() const {
	return p.id;
} // Train::getId


// TS12(a-ii). Note that the students remain blocked on the train’s entry queue ...
TrainStop * Train::embark(unsigned int studentId, unsigned int destStop, WATCard& card) {
	TrainStop * destStopAddr;
	p.sid = studentId;

	// Find a seat
	unsigned int i = 0;
	for ( ; i < p.maxNumStudents; i += 1 ) {
		if ( !( p.seats[i].isOccupied ) ) {
			p.seats[i].isOccupied = true;
			p.seats[i].card = &card;
			p.seats[i].student = &uThisTask();
			p.seats[i].destTrainStop = &destStopAddr;
			break;
		} // if
	} // for

	// TR8(c). ... blocks again “within” the train ...
	p.seats[i].condSeat.wait(destStop);

	return destStopAddr;
} // Train::embark
// .(ii-a)21ST

// S18(b-ii). ... or after being caught by a conductor and ejected from the train.
// TS4(c-ii). ... which is used as a proof-of-purchase to the  
//         	  conductor that the student has paid their fare.
// TR10(c-i). ... that students on the train have paid their fare.
// CO2(c-i). ... which lets it check all passengers on board to verify that they have a ticket.
// WC7(ii). A conductor calls paidForTicket to see the POP.
void Train::scanPassengers() {
	// no op, just triggers _Accept in TrainStop::main
} // Train::scanPassengers
// .(ii)7CW
// .(i-c)2OC
// .(i-c)01RT
// .(ii-c)4ST
// .(ii-b)81S

void Train::main() {
	bool isClockwise = (p.direction == Direction::Clockwise);
	char dir = (isClockwise) ? '<' : '>';
	p.prt.print( Printer::Kind::Train, p.id, 'S', p.curStop, dir );

	// NS4(a-ii). The ... and trains obtain(s) the list of stops
	//            from the name server by calling getStopList.
	// TR5. The train then obtains a list of stops from the NameServer.
	TrainStop ** stopList = p.nameServer.getStopList(p.id);

	for ( ;; ) {
		_Accept( ~Train ) {
			break;

		// TR10(c-ii). ... that students on the train have paid their fare.
		// CO2(c-ii). ... which lets it check all passengers on board to verify that they have a ticket.
		} or _Accept( scanPassengers ) {
			for ( unsigned int i = 0 ; i < p.maxNumStudents; i += 1 ) {
				// TS4(c-iii). ... which is used as a proof-of-purchase to the  
				//         	   conductor that the student has paid their fare.
				// WC7(iii). A conductor calls paidForTicket to see the POP.
				if ( p.seats[i].isOccupied && !(p.seats[i].card->paidForTicket()) ) {
					p.seats[i].isCaught = true;
				} // if
			} // for
		// .(ii-c)2OC
		// .(ii-c)01RT

		} _Else {
			TrainStop * curStop = stopList[p.curStop];
			unsigned int temp = p.numStudents;

			// TR7(a). At every stop, it signals any blocked students ...
			// CO3. Any passengers without a POP are summarily ejected from the train at the next stop.
			for ( unsigned int i = 0 ; i < p.maxNumStudents; i += 1 ) {	// check all seats in the train
				if ( !p.seats[i].condSeat.empty() ) {
					if ( p.seats[i].isCaught ) {
						p.seats[i].isOccupied = p.seats[i].isCaught = false;
						p.seats[i].card = nullptr;
						p.numStudents -= 1;
						// S18(b-iii). ... or after being caught by a conductor and ejected from the train.
						_Resume Ejected{} _At *(p.seats[i].student);
			// .3OC
						p.seats[i].student = nullptr;
						p.seats[i].condSeat.signalBlock();

					// TR7(b). ... who have reached their destination stop ...
					// TR8(d). ... until it reaches the appropriate train stop, ...
					} else if ( p.seats[i].condSeat.front() == p.curStop ) {
						p.seats[i].isOccupied = p.seats[i].isCaught = false;
						p.seats[i].card->resetPOP();
						p.seats[i].card = nullptr;
						p.seats[i].student = nullptr;
						*(p.seats[i].destTrainStop) = curStop;
						// TR7(c). ... so that they can leave and disembark at the station, ...
						p.seats[i].condSeat.signalBlock();
						p.numStudents -= 1;
					} // if
					// .(d)8RT
					// .(b)7RT
				} // if
			} // for
			// .(a)7RT
			unsigned int freeSpace = p.maxNumStudents - p.numStudents;
			p.prt.print( Printer::Kind::Train, p.id, 'A', p.curStop, freeSpace, temp );
			// TS12(b). ... until the train accepts their calls, ...
			// TS9. The train calls arrive, specifying its direction of travel and the maximum number of students 
			//      it can take from this stop after taking into account the number it is currently transporting.
			// TR6(i). It visits each stop in turn at every “tick” of the Timer task.
			unsigned int taken = curStop->arrive( p.id, p.direction, freeSpace );
			// TR7(d). ... and takes on waiting passengers through embark until it reaches its limit of maxNumStudents.
			//         POTENTIAL BUG: no maxNumStudents appeared!
			for ( unsigned int i = 0 ; i < taken; i += 1 ) {
				_Accept( embark ) {
					p.prt.print( Printer::Kind::Train, p.id, 'E', p.sid, p.curStop );
					p.numStudents += 1;
				} // _Accept
			} // for
			// .(b)21ST

			// TR2. Travelling in a clockwise direction means that it travels from stop 0 to stop 1, 
			//      from stop 1 to stop 2, etc, wrapping back around from stop numStops - 1 back to stop 0. 
			// TR3. Travelling in a counter-clockwise direction reverses the order of stops.
			p.curStop = (isClockwise ? (p.curStop + 1) : (p.curStop + p.numStops - 1)) % p.numStops;

		} // _Accept
	} // for

	p.prt.print( Printer::Kind::Train, p.id, 'F' );
} // Train::main

