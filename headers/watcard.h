#pragma once

#include <uFuture.h>

class WATCard {
    // WC5(b). ... which is why the TrainStop is declared a friend.
    friend _Task TrainStop;
    
    class PImpl; // *** Replace these two lines with your own implementation.
    PImpl * pimpl;

    WATCard( const WATCard & ) = delete;				// prevent copying
    WATCard & operator=( const WATCard & ) = delete;

    // WC5(a). The markPaid method is declared private to keep the student from calling it, ...
    void markPaid();
    
  public:
    typedef Future_ISM<WATCard *> FWATCard;				// future watcard pointer
    WATCard();
    ~WATCard();
    void deposit( unsigned int amount );
    void withdraw( unsigned int amount );
    unsigned int getBalance();
    bool paidForTicket();
    void resetPOP();
}; // WATCard
