#include "headers/cardoffice.h"
#include "headers/printer.h"
#include "headers/bank.h"
#include "headers/watcard.h"

#include <uPRNG.h>
#include <list>

using std::list;

#define p (*pimpl)

// WCO1. The WATCardOffice is an administrator task used by a 
//       student to transfer funds from their bank account to 
//       their WATCard to buy a train ticket.
struct Job {
    unsigned int sid, amount;
    WATCard * rCard;
    WATCard::FWATCard fCard;

    Job( unsigned int sid, unsigned int amount, WATCard * card ) :
        sid{ sid }, amount{ amount }, rCard{ card } {}
}; // Job


class WATCardOffice::PImpl {
  public:
    _Task Courier {
        Printer & prt;
        Bank & bank;
        unsigned int id;
        WATCardOffice & cardoffice;

        void main();
      public:
        Courier( Printer & prt, Bank & bank, unsigned int id, WATCardOffice & cardoffice ) :
            prt{ prt }, bank{ bank }, id{ id }, cardoffice{ cardoffice } {}
    }; // _Task Courier

    // Internal data
    Printer & prt;
    Bank & bank;
    unsigned int numCouriers;

    // Communication vars
    list<Job *> jobs;
    list<Courier *> couriers;

    PImpl( Printer & prt, Bank & bank, unsigned int numCouriers ) :
        prt{ prt }, bank{ bank }, numCouriers{ numCouriers } {}
}; // WATCardOffice::PImpl


WATCardOffice::WATCardOffice( Printer & prt, Bank & bank, unsigned int numCouriers ) {
	pimpl = new PImpl { prt, bank, numCouriers };
} // WATCardOffice::WATCardOffice


WATCardOffice::~WATCardOffice() {
	delete pimpl;
} // WATCardOffice::~WATCardOffice


// S16(iii). If their WATCard is lost, they request a replacement 
//         initialized to the maximum cost of a trip.
// WCO5(b-i). ... and sufficient funds are subsequently obtained 
//            from the bank via a courier to satisfy the create request.
// called by student
WATCard::FWATCard WATCardOffice::create( unsigned int sid, unsigned int amount ) {
    WATCard * card = new WATCard{};
    Job * job = new Job{ sid, amount, card };
    p.jobs.emplace_back( job );
    return job->fCard;
} // WATCardOffice::create
// .(i-b)5OCW
// .(iii)61S


// S15(iii). If they have insufficient funds, a request is made for  
//          the missing amount plus the maximum cost of a trip.
// WCO7(i). The WATCard office is empowered to transfer funds 
//          from a student’s bank-account to its WATCard by 
//          sending a request through a courier to the bank.
// called by student
WATCard::FWATCard WATCardOffice::transfer( unsigned int sid, unsigned int amount, WATCard * card ) {
    Job * job = new Job{ sid, amount, card };
    p.jobs.emplace_back( job );
    return job->fCard;
} // WATCardOffice::transfer
// .(i)7OCW
// .(iii)51S


// called by courier to request/return work
Job * WATCardOffice::requestWork() {
    if ( p.jobs.empty() ) return nullptr;
    Job * job = p.jobs.front();
    p.jobs.pop_front();
    return job;
} // WATCardOffice::requestWork


void WATCardOffice::PImpl::Courier::main() {
    prt.print( Printer::Kind::WATCardOfficeCourier, id, 'S' );

    for ( ;; ) {
        // WCO7(ii). The WATCard office is empowered to transfer funds 
        //           from a student’s bank-account to its WATCard by 
        //           sending a request through a courier to the bank.
        // WCO8. Each courier task calls requestWork, blocks until a 
        //       Job request is ready, and then receives the next 
        //       Job request as the result of the call.
        //       [!!! POTENTIAL BUG: NOT CONFORMING THIS CLAUSE AS COURIERS ARE NOT BLOCK WHEN JOB IS NOT READY]
        Job * job = cardoffice.requestWork();
        if ( job == nullptr ) break;   // no more work to do
        unsigned int sid = job->sid, amount = job->amount;
        WATCard * rCard = job->rCard;

        // Do work
        prt.print( Printer::Kind::WATCardOfficeCourier, id, 't', sid, amount );
        // WCO5(b-ii). ... and sufficient funds are subsequently obtained from 
        //             the bank via a courier to satisfy the create request.
        // BA4. A courier calls withdraw to transfer money on
        //      behalf of the WATCard office for a specific student.
        // BA5(a). The courier waits until enough money has been
        //         deposited, which may require multiple deposits.
        bank.withdraw( sid, amount );
        // WC3. The WATCard office calls deposit after a funds transfer.
        // WCO9. As soon as the request is satisfied (i.e., money is obtained 
        //       from the bank), the courier updates the student’s WATCard.
        rCard->deposit( amount );
        // .(ii)7OCW

        // S16(i). If their WATCard is lost, they request a replacement 
        //         initialized to the maximum cost of a trip.
        // WCO10. There is a 1 in 6 chance a courier loses a student’s WATCard after the update.
        if ( prng( 6 ) == 0 ) {     // lost ?
            prt.print( Printer::Kind::WATCardOfficeCourier, id, 'L', sid );
            // WCO11(a). When the card is lost, the exception WATCardOffice::Lost is inserted 
            //           into the future, rather than making the future available, ...
            // NOTE: Why do we need to new it? How about just a local var?
            job->fCard.exception( new WATCardOffice::Lost() );
        // .(i)61S
            // WCO11(b). ... and the current WATCard is deleted.
            // WCO12(a). The WATCard office is not responsible for deleting the cards. 
            //           That is done by ... ,or the courier when it loses a card.
            delete rCard;
        
        } else {
            job->fCard.delivery( rCard );
            prt.print( Printer::Kind::WATCardOfficeCourier, id, 'T', sid, amount );
        } // if
        delete job;
    } // for

    prt.print( Printer::Kind::WATCardOfficeCourier, id, 'F' );
} // WATCardOffice::PImpl::main


void WATCardOffice::main() {
    p.prt.print( Printer::Kind::WATCardOffice, 'S' );
    
    // WCO2. Initially, the WATCard office creates a fixed-sized courier pool 
    //       of numCouriers courier tasks to communicate with the bank.
    // WCO3. Additional couriers may not be created after the WATCardOffice begins.
    for ( unsigned int i = 0; i < p.numCouriers; i += 1 ) {
        p.couriers.push_back( new PImpl::Courier{ p.prt, p.bank, i, *this } );
    } // for
    // .20CW

    for ( ;; ) {
        _Accept( ~WATCardOffice ) {
            p.jobs.clear();
            for ( unsigned int i = 0; i < p.numCouriers; i += 1 ) {
                _Accept( requestWork );             // let couriers receive NULL as a sentinel
            } // for
            while ( ! p.couriers.empty() ) {
                assert( p.couriers.back() != nullptr );
                delete p.couriers.back();
                p.couriers.pop_back();
            } // while
            break;
        
        } or _When ( !p.jobs.empty() ) _Accept( requestWork ) {
            p.prt.print( Printer::Kind::WATCardOffice, 'W' );

        } or _Accept( create ) {
            p.prt.print( Printer::Kind::WATCardOffice, 'C', p.jobs.back()->sid, p.jobs.back()->amount );

        } or _Accept( transfer ) {
            p.prt.print( Printer::Kind::WATCardOffice, 'T', p.jobs.back()->sid, p.jobs.back()->amount );
        } // _Accept
    } // for

    p.prt.print( Printer::Kind::WATCardOffice, 'F' );
} // WATCardOffice::main

// .1OCW

