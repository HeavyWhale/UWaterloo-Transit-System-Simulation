#include <iostream>
#include <string>
#include <uPRNG.h>

using namespace std;

int main( int argc, char *argv[] ) {
    int * balances = new int[10];
    balances[3] = 1;
    std::cout << balances[3] << std::endl;
    
    int & balance = balances[3];
    balance += 3;
    std::cout << balances[3] << std::endl;
}
