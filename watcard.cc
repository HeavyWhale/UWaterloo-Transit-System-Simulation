#include "headers/watcard.h"

#define p (*pimpl)


class WATCard::PImpl {
  public:
	// Internal data
	unsigned int balance;
	bool hasPOP;

	PImpl() : balance{ 0 }, hasPOP{ false } {}
};// WATCard::PImpl


WATCard::WATCard() {
	pimpl = new PImpl {};
} // WATCard::WATCard


WATCard::~WATCard() {
	delete pimpl;
} // WATCard::~WATCard


void WATCard::deposit( unsigned int amount ) {
	p.balance += amount;
} // WATCard::deposit


void WATCard::withdraw( unsigned int amount ) {
	assert( p.balance >= amount );
	p.balance -= amount;
} // WATCard::withdraw


unsigned int WATCard::getBalance() {
	return p.balance;
} // WATCard::getBalance


bool WATCard::paidForTicket() {
	return p.hasPOP;
} // WATCard::paidForTicket


void WATCard::resetPOP() {
	p.hasPOP = false;
} // WATCard::resetPOP


void WATCard::markPaid() {
	p.hasPOP = true;
} // WATCard::markPaid
