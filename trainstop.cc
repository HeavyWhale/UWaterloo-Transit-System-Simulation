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
    // TS2. It is given its `id` and the amount to charge per stop travelled,
    //      `stopCost`, used when a student asks to buy a ticket.
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


void TrainStop::buy( unsigned int numStops, WATCard & card ) {
    // WC6(ii). A student and a train stop call getBalance to determine the balance.
    unsigned int balance = card.getBalance();
    unsigned int cost = numStops * p.stopCost;

    // "If the student has sufficient funds in the gift card or the 
    //  WATCard, then the amount is debited; otherwise, the exception 
    //  Funds is raised, which contains the amount the student needs 
    //  in order to be able to complete the payment after the card 
    //  balance is debited."
    // S15(ii). If they have insufficient funds, a request is made for  
    //          the missing amount plus the maximum cost of a trip.
    // TS5(b). ... otherwise, the exception Funds is raised, which contains the amount the student  
    //         needs in order to be able to complete the payment after the card balance is debited.
    if ( balance < cost ) {     // ? enough balance
        _Throw Funds{ cost - balance };
    // .(b)5ST
    // .(ii)51S

    // TS5(a-i). If the student has sufficient funds in the gift card 
    //           or the WATCard, then the amount is debited; ...
    } else {
        // WC4. A train stop calls withdraw and then markPaid when a fare is purchased.
        card.withdraw( cost );
    // .(i-a)5ST
        // TS4(b). ... marked internally as paid, ...
        card.markPaid();
        // .4CW
        p.prt.print( Printer::Kind::TrainStop, p.id, 'B', cost );
    } // if
} // TrainStop::buy


// TS7(b). ... which blocks the student at the stop ...
Train * TrainStop::wait( unsigned int studentId, Train::Direction direction ) {
    p.dir = direction;
    p.sid = studentId;

    // TR8(a). A student that unblocks at a train stop ...
    if ( direction == Train::Direction::Clockwise ) {
        p.condStudCW.wait();
        return p.trainCW;
    } else {
        p.condStudCCW.wait();
        return p.trainCCW;
    } // if
    // .(a)8RT
} // TrainStop::wait
// .(b)7ST


// TS13(ii). A student calls disembark to indicate that 
//          it is getting off the train at this stop.
// TS14(i). The TrainStop then blocks until tick is called. 
void TrainStop::disembark( unsigned int studentId ) {
    p.sid = studentId;
} // TrainStop::disembark
// .(a)41ST
// .(b)31ST


// TS15(ii). The Timer calls tick to advance the system clock, 
//           waking any trains blocked at this stop.	  
void TrainStop::tick() {
    // no op, just triggers _Accept in TrainStop::main
} // TrainStop::tick
// .(ii)51ST


// Train tells stop in which direction it is traveling, and the maximum 
// number of students it can take. It then blocks until the timer ticks
// over. Returns the minimum of what it can take, and the number of 
// students waiting.
// TR6(ii). It visits each stop in turn at every “tick” of the Timer task.
unsigned int TrainStop::arrive( unsigned int trainId, Train::Direction direction, unsigned int maxNumStudents ) {
    p.tid = trainId;
    p.dir = direction;
    p.maxNumStudents = maxNumStudents;
    p.arrivedTrain = (Train *)(&uThisTask());

    unsigned int numEmbarked;
    p.numEmbarked = &numEmbarked;

    // TS10(i). It then blocks until the next call to tick.
    // TS12(c). ... which will only happen after a call to tick has been made.
    p.condTrain.wait();
// .(ii)6RT

    return numEmbarked;
} // TrainStop::arrive


void TrainStop::main() {
    p.prt.print( Printer::Kind::TrainStop, p.id, 'S' );
    // NS2(a). Each TrainStop must register itself upon creation  
    //         with the name server by calling registerStop ...
    p.nameServer.registerStop( p.id );

    // TS1. A TrainStop’s function is to act as a synchronization 
    //      point between the trains and the students.
    for ( ;; ) {
        _Accept( ~TrainStop ) {
            break;

        } or _Accept( buy ) {
            // no op
        
        } or _Accept( wait ) {
            bool isClockwise = (p.dir == Train::Direction::Clockwise);
            char dir = ( isClockwise ) ? '<' : '>';
            p.prt.print( Printer::Kind::TrainStop, pimpl->id, 'W', p.sid, dir );
            if ( isClockwise ) {
                p.numWaitingCW += 1;
            } else {
                p.numWaitingCCW += 1;
            } // if
        
        // TS7(c). ... until the train travelling in the appropriate direction arrives.
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
            // TS11. Within arrive, the TrainStop unblocks the appropriate number of waiting students (accounts 
            //       for those disembarking) and these students call Train::embark to get on the train.
            unsigned int numCanTake = ( p.maxNumStudents > numWaiting ) ? numWaiting : p.maxNumStudents;
            for ( unsigned int i = 0; i < numCanTake; i += 1 ) {
                condStud.signalBlock();
        // .(c)7ST
                numWaiting -= 1;
            } // for
            *(p.numEmbarked) = numCanTake;

        // TS14(ii). The TrainStop then blocks until tick is called. 
        //           NOTE: The TrainStop implicitly accept-blocked itself. See comments about the _Accept in `supressed.h`
        } or _Accept( disembark ) {
            p.prt.print( Printer::Kind::TrainStop, p.id, 'D', p.sid );

        // TS10(ii). It then blocks until the next call to tick.
        // TS12(b). ... until the train accepts their calls, which will 
        //          only happen after a call to tick has been made.
        // TS15(iii). The Timer calls tick to advance the system clock, 
        //            waking any trains blocked at this stop.	
        // TR6(iv). It visits each stop in turn at every “tick” of the Timer task.
        } or _Accept( tick ) {          // Accept tick in the end
            p.prt.print( Printer::Kind::TrainStop, p.id, 't' );
            while ( !p.condTrain.empty() ) p.condTrain.signalBlock();
        } // _Accept
        // .(vi)6RT
        // .(iii)51ST
        // .(b)21ST
        // .(ii)01ST
    } // for
    // .6SN

    p.prt.print( Printer::Kind::TrainStop, p.id, 'F' );
} // TrainStop::main

