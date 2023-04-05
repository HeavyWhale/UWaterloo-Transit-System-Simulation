#include "headers/watcard.h"

#define p (*pimpl)


// WC1. The WATCard manages the money associated with a card 
//      and contains the “proof of purchase” (POP) for a fare.
class WATCard::PImpl {
  public:
	// Internal data
	unsigned int balance;
	// S10. Proof of purchase (POP) is encoded in the WATCard.
	bool hasPOP;

	// WC2. Its balance is set to 0 at creation.
	PImpl() : balance{ 0 }, hasPOP{ false } {}
};// WATCard::PImpl
// .1CW


WATCard::WATCard() {
	pimpl = new PImpl {};
} // WATCard::WATCard


WATCard::~WATCard() {
	delete pimpl;
} // WATCard::~WATCard


void WATCard::deposit( unsigned int amount ) {
	p.balance += amount;
} // WATCard::deposit


// TS5(a-ii). If the student has sufficient funds in the gift card 
//            or the WATCard, then the amount is debited; ...
void WATCard::withdraw( unsigned int amount ) {
	assert( p.balance >= amount );
	p.balance -= amount;
} // WATCard::withdraw
// .(ii-a)5ST


// WC6(iii). A student and a train stop call getBalance to determine the balance.
unsigned int WATCard::getBalance() {
	return p.balance;
} // WATCard::getBalance
// .(iii)6CW


// WC7(iv). A conductor calls paidForTicket to see the POP.
bool WATCard::paidForTicket() {
	return p.hasPOP;
} // WATCard::paidForTicket
// .(vi)7CW


void WATCard::resetPOP() {
	p.hasPOP = false;
} // WATCard::resetPOP


void WATCard::markPaid() {
	p.hasPOP = true;
} // WATCard::markPaid
