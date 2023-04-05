#include "headers/nameserver.h"
#include "headers/printer.h"
#include "headers/trainstop.h"

#define p (*pimpl)


class NameServer::PImpl {
  public:
	// Internal data
	Printer & prt;
	unsigned int numStops, numStudents;
	// NS1. The NameServer is a server task used to manage the train stop names.
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

// NS2(b-i). ... which stores the address of the calling task.
void NameServer::registerStop( unsigned int trainStopId ) {
	p.trainStopId = trainStopId;
	p.trainStopAddr = (TrainStop*)(&uThisTask());
} // NameServer::registerStop
// .(i-b)2SN


// NS5(b). Students call getStop to know the appropriate train 
//         stop in order to buy a ticket and embark/disembark.
TrainStop * NameServer::getStop( unsigned int studentId, unsigned int trainStopId ) {
	p.studentId = studentId;
	p.trainStopId = trainStopId;
	return p.trainStops[trainStopId];
} // NameServer::getStop
// .(b)5SN


TrainStop ** NameServer::getStopList() {
	p.getStopListCallerKind = Printer::Kind::Timer;
	return p.trainStops;
} // NameServer::getStopList


// NS4(b). The ... and trains obtain(s) the list of stops  
//         from the name server by calling getStopList.
TrainStop ** NameServer::getStopList( unsigned int trainId ) {
	p.getStopListCallerKind = Printer::Kind::Train;
	p.trainId = trainId;
	return p.trainStops;
} // NameServer::getStopList
// .(b)4SN


unsigned int NameServer::getNumStops() {
	return p.numStops;
} // NameServer::getNumStops


void NameServer::main() {
	p.prt.print( Printer::Kind::NameServer, 'S');

	// NS2(b-ii). ... which stores the address of the calling task.
	// NS3. The simulation cannot start until all of the 
	//      trains stops have registered themselves.
	for ( unsigned int i = 0; i < p.numStops; i += 1 ) {
		_Accept( registerStop ) {
			p.prt.print( Printer::Kind::NameServer, 'R', p.trainStopId );
			p.trainStops[p.trainStopId] = p.trainStopAddr;
		} // _Accept
	} // for
	// .3SN
	// .(ii-b)2SN

	// Do printing after clients get what they want
	for ( ;; ) {
		_Accept( ~NameServer ) {
			break;
		} or _Accept( getStop ) {
			p.prt.print( Printer::Kind::NameServer, 'T', p.studentId, p.trainStopId );
		
		// NS4(c). Students call getStop to know the appropriate train 
		//         stop in order to buy a ticket and embark/disembark.
		} or _Accept( getStopList ) {
			if ( p.getStopListCallerKind == Printer::Kind::Timer ) {
				p.prt.print( Printer::Kind::NameServer, 'L' );
			} else {
				p.prt.print( Printer::Kind::NameServer, 'L', p.trainId );
			} // if
		// .(c)4SN
		} or _Accept( getNumStops ) {} // _Accept
	} // for

	p.prt.print( Printer::Kind::NameServer, 'F' );
} // NameServer::main
