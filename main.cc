#include <iostream>
#include <string>
#include <uPRNG.h>

#include "headers/config.h"
#include "headers/printer.h"
#include "headers/bank.h"
#include "headers/parent.h"
#include "headers/cardoffice.h"
#include "headers/groupoff.h"
#include "headers/nameserver.h"
#include "headers/timer.h"
#include "headers/trainstop.h"
#include "headers/train.h"
#include "headers/conductor.h"
#include "headers/student.h"

using namespace std;

int main( int argc, char *argv[] ) {
	const char * infile = "lrt.config";                         // Defaults
	int seed = 0, processors = 1;                               // Defaults 

	try {
		switch (argc) {
		  case 4: 
			if ( strcmp( argv[3], "d" ) != 0 ) {  // default ?
				processors = stoi(argv[3]); if (processors <= 0) throw 1; // invalid ?
			} // if
			// FALL THROUGH
		  case 3: 
			if ( strcmp( argv[2], "d" ) != 0 ) {  // default ?
				seed = stoi(argv[2]); if (seed <= 0) throw 1;   // invalid ?
			} // if
			// FALL THROUGH
		  case 2: 
			if ( strcmp( argv[1], "d" ) != 0 ) {  // default ?
				infile = argv[1];                               // invalid ?
			} // if
			// FALL THROUGH
		  case 1:                                               // all defaults
			break;
		  default:                                              // wrong number of options
			throw 1;
		} // switch
	} catch (...) {
		cout << "Usage: " << argv[0]
			<< " [ d | file [ d | seed [ d | processors ] ] ]"
			<< endl;
		exit ( EXIT_FAILURE );                      // TERMINATE!
	} // try

	ConfigParms cParms;

	// 0. Use processors number in the following declaration placed in the 
	//    program main immediately after checking commandline arguments
	//    but before creating any tasks ...
	uProcessor p[processors - 1] __attribute__(( unused )); // create more kernel thread

	// ... and set the seed
	set_seed( seed );

	// 1. The program main starts by calling processConfigFile to read 
	//    and parse the simulation configurations.
	processConfigFile( infile, cParms );

	// 2. My driver created, in order: printer, bank, parent, WatCard office, 
	//    Groupoff, nameserver, timer, train stops, trains, conductors, and 
	//    students. The WatCard office creates the couriers.
	unsigned int maxTripCost = cParms.numStops / 2 * cParms.stopCost;

	Printer printer { cParms.numStudents, 2, cParms.numStops, cParms.numCouriers };
	Bank bank { cParms.numStudents };
	Parent parent { printer, bank, cParms.numStudents, cParms.parentalDelay, maxTripCost };
	WATCardOffice cardOffice { printer, bank, cParms.numCouriers };
	Groupoff groupoff { printer, cParms.numStudents, maxTripCost, cParms.groupoffDelay };
	NameServer nameServer { printer, cParms.numStops, cParms.numStudents };
	Timer * timer = new Timer { printer, nameServer, cParms.timerDelay };

	TrainStop * trainStops[ cParms.numStops ];
	for ( unsigned int i = 0; i < cParms.numStops; i += 1 ) {
		trainStops[i] = new TrainStop { printer, nameServer, i, cParms.stopCost };
	} // for

	Train * trains[2];
	for ( unsigned int i = 0; i < 2; i += 1 ) {
		trains[i] = new Train { printer, nameServer, i, cParms.maxNumStudents, cParms.numStops };
	} // for

	Conductor * conductors[2];
	for ( unsigned int i = 0; i < 2; i += 1 ) {
		conductors[i] = new Conductor { printer, i, trains[i], cParms.conductorDelay };
	} // for

	Student * students[ cParms.numStudents ];
	for ( unsigned int i = 0; i < cParms.numStudents; i += 1 ) {
		students[i] = new Student { 
			printer, nameServer, cardOffice, groupoff, i,
			cParms.numStops, cParms.stopCost, cParms.maxStudentDelay, 
			cParms.maxStudentTrips 
		};
	} // for

	// 3. The entire simulation must shut down in an orderly fashion 
	//    once all of the students have completed their trips.
	for ( unsigned int i = 0; i < cParms.numStudents; i += 1 ) {	// Students
		delete students[i];
	} // for

	for ( unsigned int i = 0; i < 2; i += 1 ) {						// Conductors & Trains
		delete conductors[i];
	} // for

	for ( unsigned int i = 0; i < 2; i += 1 ) {						// Conductors & Trains
		delete trains[i];
	} // for

	// NOTE: Delete timer before deleting trainStops as it calls `tick()` 
	// on trainStops. Otherwise any trains block on trainStops cannot be 
	// unblocked to terminate themselves.
	delete timer;

	for ( unsigned int i = 0; i < cParms.numStops; i += 1 ) {       // Tainstops
		delete trainStops[i];
	} // for
} // ::main
