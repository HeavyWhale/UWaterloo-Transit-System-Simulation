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

    unsigned int numTrips = prng(1, p.maxStudentTrips);
    unsigned int numStops = p.numStops;
    unsigned int maxTripCost = p.stopCost * numStops / 2;
    WATCard::FWATCard giftcard = p.groupoff.giftCard();
    WATCard::FWATCard watcard = p.cardOffice.create(p.id, maxTripCost);
    
    p.printer.print( Printer::Kind::Student, p.id, 'S', numTrips );

    for ( unsigned int i = 0; i < numStops; i += 1 ) {
        stops.push_back(i);
    } // for

    unsigned int startIndex, endIndex;
    unsigned int startStopId, endStopId;

  _Enable {
    for (unsigned int i = 0; i < numTrips; ++i) {
        yield( prng( p.maxStudentDelay ) );
        if ( i == 0 ) {
            startIndex = prng( numStops );
            // stops[startIndex] = stops[numStops - 1];
            // stops[numStops - 1] = startStopId;
        }

        while ( (endIndex = prng(numStops)) == startIndex ) {}

        osacquire( cerr ) << startIndex << " " << endIndex << endl;

        startStopId = stops[ startIndex ];
        endStopId = stops[ endIndex ];

        // endIndex = prng(numStops - 1);
        // endStopId = stops[endIndex];

        TrainStop * startStop = p.nameServer.getStop( p.id, startStopId );

        unsigned int numTripStops;
        Train::Direction trainDir;
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
        char dir = (trainDir == Train::Direction::Clockwise) ? '<' : '>';
        p.printer.print( Printer::Kind::Student, p.id, 'T', startStopId, endStopId, dir );

        bool freeride = false;
        if (numTripStops == 1) {
            if (prng(2) == 0) freeride = true;
        } else {
            if (prng(1, 10) <= 3) freeride = true;
        } // if

        unsigned int cost = p.stopCost * numTripStops;
        if ( ! freeride ) {
            for ( ;; ) {
                try {
                    _Select(giftcard) {
                        WATCard * card = giftcard();
                        startStop->buy( numTripStops, *card );
                        giftcard.reset();
                        p.printer.print( Printer::Kind::Student, p.id, 'G', cost, card->getBalance() );
                        break;
                    } or _Select(watcard) {
                        WATCard * card = watcard();
                        startStop->buy( numTripStops, *card );
                        p.printer.print( Printer::Kind::Student, p.id, 'B', cost, card->getBalance() );
                        break;                        
                    } // _Select
                } catch ( WATCardOffice::Lost &  ) {
                    p.printer.print( Printer::Kind::Student, p.id, 'L' );
                    watcard = p.cardOffice.create( p.id, maxTripCost );
                    continue;
                } catch ( TrainStop::Funds & f ) {
                    WATCard * card = watcard();
                    p.cardOffice.transfer( p.id, f.amount, card );
                    continue;
                } // try
            } // for
        } else {
            p.printer.print( Printer::Kind::Student, p.id, 'f' );
        } // if

        for ( ;; ) {
            try {
                watcard();
                break;
            } catch ( WATCardOffice::Lost & ) {
                watcard = p.cardOffice.create( p.id, maxTripCost );
                continue;
            } // try
        } // for

        try {
            WATCard * curCard = watcard();
            p.printer.print( Printer::Kind::Student, p.id, 'W', startStopId );
            Train * curTrain = startStop->wait( p.id, trainDir );
            p.printer.print( Printer::Kind::Student, p.id, 'E', curTrain->getId() );
            TrainStop * endStop = curTrain->embark( p.id, endStopId, *curCard );
            p.printer.print( Printer::Kind::Student, p.id, 'D', endStopId );
            endStop->disembark( p.id );
        } catch ( Train::Ejected & ) {
            p.printer.print( Printer::Kind::Student, p.id, 'e' );
            break;
        } // try

        // startStopId = endStopId;
        startIndex = endIndex;
        // stops[endIndex] = stops[numStops - 1];
        // stops[numStops - 1] = startStopId;
    } // for
   } // _Enable

    delete watcard;
    p.printer.print( Printer::Kind::Student, p.id, 'F' );
}
