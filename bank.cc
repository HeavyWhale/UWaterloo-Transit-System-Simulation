#include "headers/bank.h"

#define p (*pimpl)


class Bank::PImpl {
  public:
	unsigned int numStudents, * balances;

	PImpl( unsigned int numStudents ) : numStudents{ numStudents } {
		balances = new unsigned int[numStudents];
		for ( unsigned int i = 0; i < numStudents; i += 1 ) {
			balances[i] = 0;
		} // for
	} // Bank::PImpl::PImpl

	~PImpl() {
		delete [] balances;
	} // Bank::PImpl::~PImpl
}; // Bank::PImpl


Bank::Bank( unsigned int numStudents ) {
	pimpl = new PImpl { numStudents };
} // Bank::Bank


Bank::~Bank() {
	delete pimpl;
} // Bank::~Bank


void Bank::deposit( unsigned int id, unsigned int amount ) {
	p.balances[id] += amount;
} // Bank::deposit


void Bank::withdraw( unsigned int id, unsigned int amount ) {
	unsigned int & balance = p.balances[id];

	// "The courier waits until enough money has been deposited,
	//  which may require multiple deposits."
	while ( balance < amount ) {    // got enough money ?
		_Accept( deposit );         // wait parent to deposit money
	} // while

	balance -= amount;
	// std::osacquire( std::cout ) << "student:" << id << ", balance:" << balance << std::endl;
} // Bank::withdraw
