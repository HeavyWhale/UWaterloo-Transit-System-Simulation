#include "headers/cardoffice.h"
#include "headers/printer.h"
#include "headers/bank.h"
#include "headers/watcard.h"

#include <uPRNG.h>
#include <list>

using std::list;

#define p (*pimpl)


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


// called by student
WATCard::FWATCard WATCardOffice::create( unsigned int sid, unsigned int amount ) {
    WATCard * card = new WATCard{};
    Job * job = new Job{ sid, amount, card };
    p.jobs.emplace_back( job );
    return job->fCard;
} // WATCardOffice::create


// called by student
WATCard::FWATCard WATCardOffice::transfer( unsigned int sid, unsigned int amount, WATCard * card ) {
    Job * job = new Job{ sid, amount, card };
    p.jobs.emplace_back( job );
    return job->fCard;
} // WATCardOffice::transfer


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
        // "Each courier task calls requestWork, blocks until a Job 
        //  request is ready, and then receives the next Job request
        //  as the result of the call.""
        Job * job = cardoffice.requestWork();
        if ( job == nullptr ) break;   // no more work to do
        unsigned int sid = job->sid, amount = job->amount;
        WATCard * rCard = job->rCard;

        // Do work
        prt.print( Printer::Kind::WATCardOfficeCourier, id, 't', sid, amount );
        bank.withdraw( sid, amount );
        rCard->deposit( amount );

        // "There is a 1 in 6 chance a courier loses a student’s 
        //  WATCard after the update."
        if ( prng( 6 ) == 0 ) {     // lost ?
            // "When the card is lost, the exception 
            //  WATCardOffice::Lost is inserted into the future, 
            //  rather than making the future available, and the 
            //  current WATCard is deleted."
            prt.print( Printer::Kind::WATCardOfficeCourier, id, 'L', sid );
            job->fCard.exception( new WATCardOffice::Lost() );
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
    
    // Create couriers 
    for ( unsigned int i = 0; i < p.numCouriers; i += 1 ) {
        p.couriers.push_back( new PImpl::Courier{ p.prt, p.bank, i, *this } );
    }

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
