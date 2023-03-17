#include "headers/trainstop.h"
#include "headers/printer.h"
#include "headers/nameserver.h"
#include "headers/watcard.h"

#define p (*pimpl)


class TrainStop::PImpl {
  public:
	// Internal data
	Printer & prt;
    NameServer & nameServer;
	unsigned int id, stopCost, numWaitingCW = 0, numWaitingCCW = 0;

    uCondition condStudCW, condStudCCW, condTrain;

    // Communication Vars
    unsigned int numStops, sid, tid, maxNumStudents, * numEmbarked;
    Train::Direction dir;
    Train * arrivedTrain, * trainCW, * trainCCW;


	PImpl( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int stopCost ) :
		prt{ prt }, nameServer{ nameServer }, id{ id }, stopCost{ stopCost } {}
};// TrainStop::PImpl


TrainStop::Funds::Funds( unsigned int amt ) : amount{ amt } {}


TrainStop::TrainStop( Printer & prt, NameServer & nameServer, unsigned int id, unsigned int stopCost ) {
    pimpl = new PImpl { prt, nameServer, id, stopCost };
} // TrainStop::TrainStop


TrainStop::~TrainStop() {
	delete pimpl;
} // TrainStop::~TrainStop


_Nomutex unsigned int TrainStop::getId() const {
    return p.id;
} // TrainStop::getId


// Student uses card to buy a ticket for numStops stops.
void TrainStop::buy( unsigned int numStops, WATCard & card ) {
    unsigned int balance = card.getBalance();
    unsigned int cost = numStops * p.stopCost;

    // "If the student has sufficient funds in the gift card or the 
    //  WATCard, then the amount is debited; otherwise, the exception 
    //  Funds is raised, which contains the amount the student needs 
    //  in order to be able to complete the payment after the card 
    //  balance is debited."
    if ( balance < cost ) {     // ? enough balance
        _Throw Funds{ cost - balance };
    } else {
        card.withdraw( cost );
        p.prt.print( Printer::Kind::TrainStop, p.id, 'B', cost );
    } // if
} // TrainStop::buy

// Student waits for a train traveling in the specified direction.
Train * TrainStop::wait( unsigned int studentId, Train::Direction direction ) {
    p.dir = direction;
    p.sid = studentId;

    // "A student calls wait, which blocks the student at the stop 
    //  until the train travelling in the appropriate direction arrives."
    if ( direction == Train::Direction::Clockwise ) {
        p.condStudCW.wait();
        return p.trainCW;
    } else {
        p.condStudCCW.wait();
        return p.trainCCW;
    } // if
} // TrainStop::wait

// Student getting off the train at this stop "disembarks" at this stop.				
void TrainStop::disembark( unsigned int studentId ) {
    p.sid = studentId;
} // TrainStop::disembark

// Timer notifies stop that timer ticked.	  
void TrainStop::tick() {
    // no op, just triggers _Accept in TrainStop::main
} // TrainStop::tick

// Train tells stop in which direction it is traveling, and the maximum 
// number of students it can take. It then blocks until the timer ticks
// over. Returns the minimum of what it can take, and the number of 
// students waiting.
unsigned int TrainStop::arrive( unsigned int trainId, Train::Direction direction, unsigned int maxNumStudents ) {
    p.tid = trainId;
    p.dir = direction;
    p.maxNumStudents = maxNumStudents;
    p.arrivedTrain = (Train *)(&uThisTask());

    unsigned int numEmbarked;
    p.numEmbarked = &numEmbarked;

    p.condTrain.wait();

    return numEmbarked;
} // TrainStop::arrive


void TrainStop::main() {
    p.prt.print( Printer::Kind::TrainStop, p.id, 'S' );
    p.nameServer.registerStop( p.id );

    for ( ;; ) {
        _Accept( ~TrainStop ) {
            break;

        } or _Accept( buy ) {

        } or _Accept( wait ) {
            bool isClockwise = (p.dir == Train::Direction::Clockwise);
            char dir = ( isClockwise ) ? '<' : '>';
            p.prt.print( Printer::Kind::TrainStop, pimpl->id, 'W', p.sid, dir );
            if ( isClockwise ) {
                p.numWaitingCW += 1;
            } else {
                p.numWaitingCCW += 1;
            } // if

        } or _Accept( arrive ) {
            bool isClockwise = (p.dir == Train::Direction::Clockwise);
            unsigned int & numWaiting = isClockwise ? p.numWaitingCW : p.numWaitingCCW;
            uCondition & condStud = isClockwise ? p.condStudCW : p.condStudCCW;
            if ( isClockwise ) {
                p.trainCW = p.arrivedTrain;
            } else {
                p.trainCCW = p.arrivedTrain;
            } // if
            p.prt.print( Printer::Kind::TrainStop, pimpl->id, 'A', p.tid, p.maxNumStudents, numWaiting );
            unsigned int numCanTake = ( p.maxNumStudents > numWaiting ) ? numWaiting : p.maxNumStudents;
            for ( unsigned int i = 0; i < numCanTake; i += 1 ) {
                condStud.signalBlock();
                numWaiting -= 1;
            } // for
            *(p.numEmbarked) = numCanTake;

        } or _Accept( disembark ) {
            p.prt.print( Printer::Kind::TrainStop, p.id, 'D', p.sid );

        } or _Accept( tick ) {          // Accept tick in the end
            p.prt.print( Printer::Kind::TrainStop, p.id, 't' );
            while ( !p.condTrain.empty() ) p.condTrain.signalBlock();
        } // _Accept
    } // for

    p.prt.print( Printer::Kind::TrainStop, p.id, 'F' );
} // TrainStop::main

