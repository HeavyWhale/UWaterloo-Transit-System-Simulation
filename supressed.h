#include <ostream>
#define _Resume 
#define _Throw throw
#define _At ;
#define _Enable
// A resumption handler is, in effect, a nested routine called from 
// the raise site when an exception is propagated with resumption; 
// when the resumption handler completes, control returns back to 
// the point of the raise.
#define _CatchResume catch
#define _Coroutine class

/* For A3 */
#define TYPE int
#define SENTINEL -1
#define NOBUSY

class uFile {
    public:
        uFile() {}
        class Failure {
            Failure() {}
        };
};

class uProcessor {
    public:
        uProcessor() {}
};

void resume() {}
void resumer() {}
void suspend() {}
void malloc_stats() {}
void yield( unsigned int i ) { (int)i; }

// #include <uC++.h>

/* For Midterm */
// #define ACTOR
// #define _Actor class
// #define Allocation uActor::Allocation


/* For A4, A5 */
// #define AUTO
#define _Monitor class
#define _Cormonitor class
#define _Finally catch(...)
#define _Nomutex
#define _Event class

// When a _Accept statement is executed, the acceptor is blocked and pushed 
// on the top of the implicit acceptor/signalled stack and a task is scheduled 
// from the mutex queue for the specified mutex member. If there is no 
// outstanding call to that mutex member, the acceptor is accept-blocked 
// until a call is made.
#define _Accept if
#define or else 
#define _Else else
#define _Task class

_Event uMutexFailure {
  public:
	_Event RendezvousFailure {};
}; // uMutexFailure

/* For A6 */
#define _When if

// µC++ provides a select statement to handle heterogeneous future selection 
// by waiting for one or more available futures based on a logical 
// selection-criteria. The selector-expression must be satisfied before 
// execution continues.
#define _Select if


struct uCondition {
public:
    uCondition();
    void wait();
    // When waiting, it is possible to optionally store an integer value with 
    // a waiting task on a condition queue by passing an argument to wait.
    // The integer value can be accessed by other tasks through the uCondition 
    // member routine `front`. This value can be used to provide more precise 
    // information about a waiting task than can be inferred from its presence 
    // on a particular condition variable.
    void wait(int);
    int front();
    void signal();
    void signalBlock();
    bool empty();
};


// The implicit-storage-management future (Future_ISM) 
// automatically allocates the required storage and 
// automatically frees the storage when the future is 
// no longer in use. [p.585 @ uCplusplusbook]
template<class T> 
class Future_ISM {
public:
    Future_ISM();
    T operator ()();
    // mark future as empty (for reuse)
    void reset();
    bool available();
    operator bool() const;
    bool exception(...);
    bool delivery(T result);
};

struct uBaseTask;

uBaseTask & uThisTask();

unsigned int prng(...);
void set_seed(...);
std::ostream osacquire(std::ostream&);

// #include "uC++.h"

