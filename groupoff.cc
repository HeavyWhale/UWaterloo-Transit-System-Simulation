#include "headers/groupoff.h"
#include "headers/watcard.h"
#include "headers/printer.h"

#include <uPRNG.h>
#include <vector>

using std::vector;

#define p (*pimpl)


class Groupoff::PImpl {
  public:
	// Internal data
	Printer & prt;
	unsigned int numStudents, maxTripCost, groupoffDelay;

	// Communication vars
	// INVARIANT: fCards.size() + rCards.size() == numStudents (normally)	
	vector<WATCard::FWATCard> fCards;	// list of unassigned future cards
	vector<WATCard *> rCards;			// list of assigned real cards
	unsigned int tail = 0;

	PImpl( Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay ) :
		prt{ prt }, numStudents{ numStudents }, maxTripCost{ maxTripCost }, groupoffDelay{ groupoffDelay }
	{
		for ( unsigned int i = 0; i < numStudents; i += 1 ) {
			fCards.push_back( WATCard::FWATCard{} );
		} // for
	} // PImpl::PImpl

	~PImpl() {
		// S19(b). ... but that the Groupff is responsible for deleting the gift cards.
		// GR8. The Groupoff deletes all gift cards as part of its termination process.
		for ( unsigned int i = 0; i < rCards.size(); i += 1 ) {
			delete rCards[i];
		} // for
		// .8RG
		// .(b)91S
	} // PImpl::~PImpl
};// Groupoff::PImpl


Groupoff::Groupoff( Printer & prt, unsigned int numStudents, unsigned int maxTripCost, unsigned int groupoffDelay ) {
	pimpl = new PImpl { prt, numStudents, maxTripCost, groupoffDelay };
} // Groupoff::Groupoff


Groupoff::~Groupoff() {
	delete pimpl;
} // Groupoff::~Groupoff


WATCard::FWATCard Groupoff::giftCard() {
	return p.fCards[p.tail];
} // Groupoff::giftCard


void Groupoff::main() {
	p.prt.print( Printer::Kind::Groupoff, 'S' );
	
	// GR1(i). The Groupoff task begins by accepting a call from each student to obtain a future gift-card.
	for ( ; p.tail < p.numStudents; p.tail += 1 ) {
		_Accept( giftCard );
	} // for
	// .(i)1RG

	// GR2. Then Groupoff periodically puts a real WATCard with
	//      value maxTripCost into a random future gift-card.
	// GR5. The Groupoff loops until all the future gift-cards are 
	//      assigned a real WATCard or a call to its destructor occurs
	// GR7. Note, this use of _Else is not busy waiting because there are a finite number of students.
	for ( ; p.fCards.size() > 0 ; ) {
		_Accept( ~Groupoff ) {
			break;

		// GR6. Since it must not block on the destructor call, it is necessary 
		//      to use a terminating _Else on the accept statement.
		} _Else {
			// GR4. Before each future gift-card is assigned a real WATCard, 
			//      Groupoff yields for groupoffDelay times (not random).
			yield( p.groupoffDelay );
			unsigned int pos = prng( p.fCards.size() );
			WATCard * card = new WATCard{};
			card->deposit( p.maxTripCost );
			p.rCards.push_back( card );

			// GR3. A future gift-card is assigned only once per student.
			p.fCards[pos].delivery( card );
			p.prt.print( Printer::Kind::Groupoff, 'D', p.maxTripCost );
			p.fCards.erase( p.fCards.begin() + pos );
		} // _Accept
	} // for
	// .5RG
	// .2RG

	p.prt.print( Printer::Kind::Groupoff, 'F' );
} // Groupoff::main
