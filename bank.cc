#include "headers/bank.h"

#define p (*pimpl)


// BA1. The Bank is a monitor, which behaves like a server, that 
//     manages student-account information for all students.
class Bank::PImpl {
  public:
	unsigned int numStudents, * balances;

	PImpl( unsigned int numStudents ) : numStudents{ numStudents } {
		balances = new unsigned int[numStudents];
		for ( unsigned int i = 0; i < numStudents; i += 1 ) {
			// BA2. Each student’s account initially starts with a balance of $0.
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


// BA3(b). The parent calls deposit to endow gifts to a specific student.
void Bank::deposit( unsigned int id, unsigned int amount ) {
	p.balances[id] += amount;
} // Bank::deposit
// .(b)3AB


void Bank::withdraw( unsigned int id, unsigned int amount ) {
	unsigned int & balance = p.balances[id];

	// BA5(b). The courier waits until enough money has been deposited, which may require multiple deposits.
	while ( balance < amount ) {    // got enough money ?
		_Accept( deposit );         // wait parent to deposit money
	} // while
	// .(b)5AB

	balance -= amount;
} // Bank::withdraw


// .1AB

