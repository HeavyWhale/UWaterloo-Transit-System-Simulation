#include "headers/nameserver.h"
#include "headers/printer.h"
#include "headers/trainstop.h"

#define p (*pimpl)


class NameServer::PImpl {
  public:
	// Internal data
	Printer & prt;
	unsigned int numStops, numStudents;
	TrainStop ** trainStops;

	// Communication vars
	unsigned int studentId, trainStopId, trainId;
	TrainStop * trainStopAddr;
	Printer::Kind getStopListCallerKind;

	PImpl( Printer & prt, unsigned int numStops, unsigned int numStudents ) :
		prt{ prt }, numStops{ numStops }, numStudents{ numStudents },
		trainStops{ new TrainStop * [numStops] } {}

	~PImpl() {
		delete [] trainStops;           // The pointed trainStops are deleted in ::main
	} // PImpl::~PImpl
};// NameServer::PImpl


NameServer::NameServer( Printer & prt, unsigned int numStops, unsigned int numStudents ) {
	pimpl = new PImpl { prt, numStops, numStudents };
} // NameServer::NameServer


NameServer::~NameServer() {
	delete pimpl;
} // NameServer::~NameServer


void NameServer::registerStop( unsigned int trainStopId ) {
	p.trainStopId = trainStopId;
	p.trainStopAddr = (TrainStop*)(&uThisTask());
} // NameServer::registerStop


TrainStop * NameServer::getStop( unsigned int studentId, unsigned int trainStopId ) {
	p.studentId = studentId;
	p.trainStopId = trainStopId;
	return p.trainStops[trainStopId];
} // NameServer::getStop


TrainStop ** NameServer::getStopList() {
	p.getStopListCallerKind = Printer::Kind::Timer;
	return p.trainStops;
} // NameServer::getStopList


TrainStop ** NameServer::getStopList( unsigned int trainId ) {
	p.getStopListCallerKind = Printer::Kind::Train;
	p.trainId = trainId;
	return p.trainStops;
} // NameServer::getStopList


unsigned int NameServer::getNumStops() {
	return p.numStops;
} // NameServer::getNumStops


void NameServer::main() {
	p.prt.print( Printer::Kind::NameServer, 'S');

	// "Each TrainStop must register itself upon creation with the name 
	//  server by calling registerStop, which stores the address of the 
	//  calling task."
	for ( unsigned int i = 0; i < p.numStops; i += 1 ) {
		_Accept( registerStop ) {
			p.prt.print( Printer::Kind::NameServer, 'R', p.trainStopId );
			p.trainStops[p.trainStopId] = p.trainStopAddr;
		} // _Accept
	} // for

	// Do printing after clients get what they want
	for ( ;; ) {
		_Accept( ~NameServer ) {
			break;
		} or _Accept( getStop ) {
			p.prt.print( Printer::Kind::NameServer, 'T', p.studentId, p.trainStopId );
		} or _Accept( getStopList ) {
			if ( p.getStopListCallerKind == Printer::Kind::Timer ) {
				p.prt.print( Printer::Kind::NameServer, 'L' );
			} else {
				p.prt.print( Printer::Kind::NameServer, 'L', p.trainId );
			} // if
		} or _Accept( getNumStops ) {} // _Accept
	} // for

	p.prt.print( Printer::Kind::NameServer, 'F' );
} // NameServer::main
