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
		// "Train 0 travels in a clockwise direction starting at stop 0,
		//  while train 1 travels in a counter-clockwise direction 
		//  starting at stop ⌈numStops/2⌉"
		if ( id == 0 ) {
			direction = Direction::Clockwise;
			curStop = 0;
		} else {
			direction = Direction::CounterClockwise;
			curStop = numStops / 2 + (( numStops % 2 > 0 ) ? 1 : 0);
		} // if
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

	p.seats[i].condSeat.wait(destStop);

	return destStopAddr;
} // Train::embark


void Train::scanPassengers() {
	// no op, just triggers _Accept in TrainStop::main
} // Train::scanPassengers


void Train::main() {
	bool isClockwise = (p.direction == Direction::Clockwise);
	char dir = (isClockwise) ? '<' : '>';
	p.prt.print( Printer::Kind::Train, p.id, 'S', p.curStop, dir );

	TrainStop ** stopList = p.nameServer.getStopList(p.id);

	for ( ;; ) {
		_Accept( ~Train ) {
			break;

		} or _Accept( scanPassengers ) {
			for ( unsigned int i = 0 ; i < p.maxNumStudents; i += 1 ) {
				if ( p.seats[i].isOccupied && !(p.seats[i].card->paidForTicket()) ) {
					p.seats[i].isCaught = true;
				} // if
			} // for

		} _Else {
			TrainStop * curStop = stopList[p.curStop];
			unsigned int temp = p.numStudents;

			for ( unsigned int i = 0 ; i < p.maxNumStudents; i += 1 ) {
				if ( !p.seats[i].condSeat.empty() ) {
					if ( p.seats[i].isCaught ) {
						p.seats[i].isOccupied = p.seats[i].isCaught = false;
						p.seats[i].card = nullptr;
						p.numStudents -= 1;
						_Resume Ejected{} _At *(p.seats[i].student);
						p.seats[i].student = nullptr;
						p.seats[i].condSeat.signalBlock();

					} else if ( p.seats[i].condSeat.front() == p.curStop ) {
						p.seats[i].isOccupied = p.seats[i].isCaught = false;
						p.seats[i].card->resetPOP();
						p.seats[i].card = nullptr;
						p.seats[i].student = nullptr;
						*(p.seats[i].destTrainStop) = curStop;
						p.seats[i].condSeat.signalBlock();
					} // if
				} // if
			} // for
			unsigned int freeSpace = p.maxNumStudents - p.numStudents;
			p.prt.print( Printer::Kind::Train, p.id, 'A', p.curStop, freeSpace, temp );
			unsigned int taken = curStop->arrive( p.id, p.direction, freeSpace );
			for ( unsigned int i = 0 ; i < taken; i += 1 ) {
				_Accept( embark ) {
					p.prt.print( Printer::Kind::Train, p.id, 'E', p.sid, p.curStop );
					p.numStudents += 1;
				} // _Accept
			} // for

			if ( isClockwise ) {
				if ( p.curStop == p.numStops - 1 ) {
					p.curStop = 0;
				} else {
					p.curStop += 1;
				}
			} else {
				if ( p.curStop == 0 ) {
					p.curStop = ceil( p.numStops / 2 );
				} else {
					p.curStop -= 1;
				}
			}

		} // _Accept
	} // for

	p.prt.print( Printer::Kind::Train, p.id, 'F' );
} // Train::main

