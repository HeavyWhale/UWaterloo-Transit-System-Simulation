#include "headers/student.h"
#include "headers/groupoff.h"
#include "headers/printer.h"
#include "headers/watcard.h"
#include "headers/cardoffice.h"
#include "headers/train.h"
#include "headers/trainstop.h"
#include "headers/nameserver.h"

#include <vector>
#include <uPRNG.h>
#include <iostream>

using namespace std;

#define p (*pimpl)

class Student::PImpl {
  public:
    Printer & printer;
    NameServer & nameServer;
    WATCardOffice & cardOffice;
    Groupoff & groupoff;
    unsigned int id;
    unsigned int numStops;
    unsigned int stopCost;
    unsigned int maxStudentDelay;
    unsigned int maxStudentTrips;

    PImpl( Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, 
        unsigned int id, unsigned int numStops, unsigned int stopCost, 
        unsigned int maxStudentDelay, unsigned int maxStudentTrips ) : 
        printer(prt), nameServer(nameServer), cardOffice(cardOffice), groupoff(groupoff),
        id(id), numStops(numStops), stopCost(stopCost), maxStudentDelay(maxStudentDelay), 
        maxStudentTrips(maxStudentTrips) {}
};// Student::PImpl

Student::Student(Printer & prt, NameServer & nameServer, WATCardOffice & cardOffice, Groupoff & groupoff, 
    unsigned int id, unsigned int numStops, unsigned int stopCost, 
    unsigned int maxStudentDelay, unsigned int maxStudentTrips) 
{
    pimpl = new PImpl{ prt, nameServer, cardOffice, groupoff, id, numStops,
        stopCost, maxStudentDelay, maxStudentTrips };
} // Student::Student

Student::~Student() {
    delete pimpl;
} // Student::~Student

void Student::main() {
    std::vector<unsigned int> stops;

    // S2(a). A Student’s function is to make a random number of train trips in the range [1, maxStudentTrips], ...
    unsigned int numTrips = prng(1, p.maxStudentTrips);
    unsigned int numStops = p.numStops;
    unsigned int maxTripCost = p.stopCost * numStops / 2;
    // S2(b-i). ... paying for a trip with either their gift card or their WATcard.
    // GR1(i). The Groupoff task begins by accepting a call from each student to obtain a future gift-card.
    WATCard::FWATCard fGiftcard = p.groupoff.giftCard();
    // S6(b-i). ... initialized to the maximum cost of a trip, ...
    // WCO4. A student performs an asynchronous call to create a “real” WATCard with an initial balance.
    // WCO5(a). A future WATCard is returned and ...
    WATCard::FWATCard fWatcard = p.cardOffice.create(p.id, maxTripCost);
    
    p.printer.print( Printer::Kind::Student, p.id, 'S', numTrips );

    for ( unsigned int i = 0; i < numStops; i += 1 ) {
        stops.push_back(i);
    } // for

    unsigned int startIndex, endIndex;
    unsigned int startStopId, endStopId;

// TS6(i). Note that, at any point, use of the card may end up with it being lost.
_Enable {
    // S18(a): A student terminates after completing all of their trips ...
    for (unsigned int i = 0; i < numTrips; ++i) {
        // S3. Before each trip is started, the student delays a random amount of time [0, maxStudentDelay] by calling yield.
        yield( prng( p.maxStudentDelay ) );
        // S4. The first trip is between two randomly-selected distinct stops with id numbers in the range [0, numStops),
        //     i.e., the start and end destinations must be different from each other.
        if ( i == 0 ) {
            startIndex = prng( numStops );
            // stops[startIndex] = stops[numStops - 1];
            // stops[numStops - 1] = startStopId;
        }

        // S5. Each subsequent trip uses the previous ending stop as the new start stop, and picks a new random distinct ending stop.
        while ( (endIndex = prng(numStops)) == startIndex ) {}
        // .4S

        osacquire( cerr ) << startIndex << " " << endIndex << endl;

        // The variables `startIndex` and `endIndex` seems redundant. `startStopId` and `endStopId` are sufficient.
        startStopId = stops[ startIndex ];
        endStopId = stops[ endIndex ];

        // S6. The student then obtains the location of the starting stop from the name server.
        //     Explaination: returns the actual stop (its pointer) determined by stop id
        // NS5(a). Students call getStop to know the appropriate train stop in order to buy a ticket and embark/disembark.
        TrainStop * startStop = p.nameServer.getStop( p.id, startStopId );

        // S13(i). To save money, it is in the student’s best interest to take the train  
        //          that travels the fewest number of stops to reach their destination.
        unsigned int numTripStops;
        Train::Direction trainDir;
        // S14. If the number of stops is the same in either direction, 
        //      the student picks the clockwise direction of travel.
        if (startStopId <= endStopId) {
            if (endStopId - startStopId > startStopId + numStops - endStopId) {
                numTripStops = startStopId + numStops - endStopId;
                trainDir = Train::Direction::CounterClockwise;     
            } else {
                numTripStops = endStopId - startStopId;
                trainDir = Train::Direction::Clockwise;    
            }   
        } else {
            if (startStopId - endStopId >= endStopId + numStops - startStopId) {
                numTripStops = endStopId + numStops - startStopId;
                trainDir = Train::Direction::Clockwise;    
            } else {
                numTripStops = startStopId - endStopId;
                trainDir = Train::Direction::CounterClockwise;
            }  
        } // if
        // .41S
        // .(i)31S

        char dir = (trainDir == Train::Direction::Clockwise) ? '<' : '>';
        p.printer.print( Printer::Kind::Student, p.id, 'T', startStopId, endStopId, dir );

        // S11. However, occasionally a student is tempted to avoid paying.
        bool freeride;
        // S12(a). If the trip is only 1 stop, there is a 50% chance a student attempts to ride without paying; ...
        if (numTripStops == 1) {
            freeride = (prng(2) == 0) ? true : false;
        
        // S12(b). ... for all other distances, there is a 30% chance.
        } else {
            freeride = (prng(1, 10) <= 3) ? true : false;
        } // if

        unsigned int cost = p.stopCost * numTripStops;
        if ( ! freeride ) {
            for ( ;; ) {
                // TS6(ii). Note that, at any point, use of the card may end up with it being lost.
                try {
                    // S2(b-ii). ... paying for a trip with either their gift card or their WATcard.
                    //      "Select statement waits for one or more heterogeneous futures based on logical selection criteria." 
                    //      [p.177 @ Course_Notes_W22]
                    // S7(a,c). Students use either their gift card or their WATCard, (b) ..., 
                    //          to pay the heavilysubsidized fare, which is based upon the number of stops travelled.
                    // S8(a). First choice of use is the gift card, ...
                    _Select( fGiftcard ) {
                        WATCard * rCard = fGiftcard(); // a functor call to the WATCard future, may wait for the actual WATCard to be ready.
                        // S15(i-1). If they have insufficient funds, a request is made for  
                        //           the missing amount plus the maximum cost of a trip.
                        // TS3(i). A student initially calls the buy method, passing in how many 
                        //         stops they are travelling and their Gift/WAT card in payment.
                        // TS4(a-i). The buy method ~returns the card~, ...
                        // WC6(i). A student and a train stop call getBalance to determine the balance.
                        startStop->buy( numTripStops, *rCard );
                        // S9. Note that in order to keep things simple, a gift card can only be used once, 
                        //     even if there are still funds left on it i.e. they cannot be transferred to 
                        //     a WATCard or used for another trip.
                        // S17. Once the gift card has been used once, it is reset since there’s no 
                        //      mechanism to transfer any remaining balance to the student’s WATCard.
                        fGiftcard.reset();
                        p.printer.print( Printer::Kind::Student, p.id, 'G', cost, rCard->getBalance() );
                        break;
                    
                    // S8(b). ... then the WATCard.
                    } or _Select( fWatcard ) {
                        WATCard * rCard = fWatcard(); // a functor call to the WATCard future, may wait for the actual WATCard to be ready.
                        // S15(i-2). If they have insufficient funds, a request is made for  
                        //           the missing amount plus the maximum cost of a trip.
                        // TS3(ii). A student initially calls the buy method, passing in how many 
                        //          stops they are travelling and their Gift/WAT card in payment.
                        // TS4(a-ii). The buy method ~returns the card~, ...
                        startStop->buy( numTripStops, *rCard );
                        p.printer.print( Printer::Kind::Student, p.id, 'B', cost, rCard->getBalance() );
                        break;                        
                    } // _Select
                    // .(c,a)7S
                    // .(ii-b)2S
                
                // S16(ii). If their WATCard is lost, they request a replacement 
                //          initialized to the maximum cost of a trip.
                // TS6(iii). Note that, at any point, use of the card may end up with it being lost.
                } catch ( WATCardOffice::Lost &  ) {
                    p.printer.print( Printer::Kind::Student, p.id, 'L' );
                    fWatcard = p.cardOffice.create( p.id, maxTripCost );
                    continue;
                // .(iii)6ST
                // .(ii)61S

                // S15(ii). If they have insufficient funds, a request is made for  
                //          the missing amount plus the maximum cost of a trip.
                } catch ( TrainStop::Funds & f ) {
                    WATCard * rCard = fWatcard();
                    // WCO6. A student performs an asynchronous call to transfer when its 
                    //       WATCard indicates there is insufficient funds to buy a ticket.
                    p.cardOffice.transfer( p.id, f.amount + maxTripCost, rCard );
                    continue;
                } // try
                // .(ii)51S
            } // for
        } else {
            p.printer.print( Printer::Kind::Student, p.id, 'f' );
        } // if

        // TR9. Note that even if a student is cheating, it still needs to present a WATCard 
        //      to embark so that the conductor can check for proof of purchase, which requires 
        //      the future WATCard to be accessed, and thus having the potential of being lost.
        /* MAJOR FIX OF HANDLING THE TS6 and TR9 CLAUSES
        for ( ;; ) {
            try {
                watcard();
                break;
            } catch ( WATCardOffice::Lost & ) {
                watcard = p.cardOffice.create( p.id, maxTripCost );
                continue;
            } // try
        } // for
        */

        WATCard * rCard;
        try {
            rCard = fWatcard();
            p.printer.print( Printer::Kind::Student, p.id, 'W', startStopId );
            // S13(ii). To save money, it is in the student’s best interest to take the train  
            //          that travels the fewest number of stops to reach their destination.
            // TS7(a). A student calls wait, ...
            Train * curTrain = startStop->wait( p.id, trainDir );
            p.printer.print( Printer::Kind::Student, p.id, 'E', curTrain->getId() );
            // TS8. Upon returning from the wait call, the student calls the train’s embark method.
            // TS12(a-i). Note that the students remain blocked on the train’s entry queue ...
            // TR8(b). ... calls embark and ...
            TrainStop * endStop = curTrain->embark( p.id, endStopId, *rCard );
            p.printer.print( Printer::Kind::Student, p.id, 'D', endStopId );
            // TS13(i). A student calls disembark to indicate that it is getting off the train at this stop.
            // TR8(e). ... at which point they must call the disembark method for the new TrainStop.
            endStop->disembark( p.id );
        
        } _CatchResume ( WATCardOffice::Lost & ) {
            fWatcard = p.cardOffice.create( p.id, maxTripCost );
            rCard = fWatcard();
        
        // S18(b-iv). ... or after being caught by a conductor and ejected from the train.
        } catch ( Train::Ejected & ) {
            p.printer.print( Printer::Kind::Student, p.id, 'e' );
            break;
        } // try
        // .(vi-b)81S
        // .9RT

        startIndex = endIndex;
    } // for
    // .(a)81S
} // _Enable

    // S19(a). Note that they will need to delete their WATCard 
    //        as part of their termination process, ...
    // WCO12(b). The WATCard office is not responsible for deleting 
    //           the cards. That is done by the students ...
    // delete watcard; // BUG FIX: watcard is a future, not a pointer.
    if (fWatcard.available()) {
        WATCard * rCard = fWatcard();
        delete rCard;
    } // if
    // .(b)21OCW
    // .(a)91S
    p.printer.print( Printer::Kind::Student, p.id, 'F' );
} // Student::main
