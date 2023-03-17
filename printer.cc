#include "headers/printer.h"

#include <iostream>
#include <string>

using namespace std;

#define p (*pimpl)
#define KIND_MAX 10

struct ColumnInfo {
	Printer::Kind kind;
	char state, c;
	unsigned int lid, oid, value1, value2;

	bool isTimer, isFlushed;
};

class Printer::PImpl {
 public:
	unsigned int numStudents, numTrains, numStops, numCouriers;
	int total, trainOffset, condOffset, stopOffset, studOffset;
	struct ColumnInfo * info;

	PImpl(unsigned int numStudents, unsigned int numTrains, unsigned int numStops, unsigned int numCouriers ):
		numStudents{ numStudents }, numTrains{ numTrains }, numStops{ numStops }, numCouriers{ numCouriers }
		{
			total = KIND_MAX + numTrains * 2 + numStops + numStudents + numCouriers - 5;
			info = new ColumnInfo[total];
			trainOffset = numTrains - 1;
			condOffset = trainOffset + numTrains - 1;
			stopOffset = condOffset + numStops - 1;
			studOffset = stopOffset + numStudents - 1;

			cout << "Parent\t" << "Gropoff\t" << "WATOff\t" << "Names\t" << "Timer\t";

			for ( int i = 0; i < KIND_MAX; ++i ) {
				string out = "";
				int numKind = 0;
				int kindOffset = 0;

				if ( i <= Kind::Timer ) {
					info[i].isFlushed = true;
					continue;
				} // if

				switch ( i ) {
					case Kind::Train: 
						out = "Train"; numKind = numTrains; kindOffset = 0; break;
					case Kind::Conductor: 
						out = "Cond"; numKind = numTrains; kindOffset = trainOffset; break;
					case Kind::TrainStop:
						out = "Stop"; numKind = numStops; kindOffset = condOffset; break;
					case Kind::Student:
						out = "Stud"; numKind = numStudents; kindOffset = stopOffset; break;
					case Kind::WATCardOfficeCourier:
						out = "WCour"; numKind = numCouriers; kindOffset = studOffset; break;
					default:
						cerr << "ERROR" << endl;
				} // switch

				for ( int j = 0; j < numKind; ++j ) {
					info[i + kindOffset + j].isFlushed = true;
					cout << out << j;
					if ( i == WATCardOfficeCourier ) {
						if ( j < numKind - 1 ) cout << '\t';
						else cout << endl;
					} else {
						cout << "\t";
					} // if
				} // if
			} // for

			for ( int i = 0; i < total; i += 1 ) {
				cout << "*******";
				if (i < total - 1) cout << "\t";
				else cout << endl;
			} // for
		}

	~PImpl() {
		flush();
		cout << "***********************" << endl;
		delete [] info;
	}


	int  getOffset( Kind kind ) {
		if ( kind == Kind::Conductor ) return trainOffset;
		else if ( kind == Kind::TrainStop ) return condOffset;
		else if ( kind == Kind::Student ) return stopOffset;
		else if ( kind == Kind::WATCardOfficeCourier ) return studOffset;
		else return 0;
	}

	void flush() {
		int index = total - 1;
		for ( int i = total - 1; i >= 0; i -= 1 ) {
			if ( info[i].isFlushed == false ) {
				index = i;
				break;
			}
		}

		for ( int i = 0; i <= index; i += 1 ) {
			if ( info[i].isFlushed == true ) {
				if ( i != index) cout << '\t';
				continue;
			}

			switch ( info[i].state ) {
				case 'S':
					if ( info[i].kind == Kind::Train )
						cout << "S" << info[i].value1 << "," << info[i].c;
					else if ( info[i].kind == Kind::Student )
						cout << "S" << info[i].value1;
					else cout << "S"; break;
				case 'B':
					if ( info[i].kind == Kind::TrainStop )
						cout << "B" << info[i].value1;
					else cout << "B" << info[i].value1 << "," << info[i].value2;
					break;
				case 'D':
					if ( info[i].kind == Kind::Parent )
						cout << "D" << info[i].value1 << "," << info[i].value2;
					else cout << "D" << info[i].value1; break;
				case 'A':
					cout << "A" << info[i].oid << "," << info[i].value1 << "," << info[i].value2; break;
				case 'C':
					cout << "C" << info[i].value1 << "," << info[i].value2; break;
				case 'G': cout << "G" << info[i].value1 << "," << info[i].value2; break;
				case 'W':
					if ( info[i].kind == Kind::WATCardOffice) cout << "W";
					else if ( info[i].kind == Kind::TrainStop )
						cout << "W" << info[i].value1 << "," << info[i].c;
					else if ( info[i].kind == Kind::Student )
						cout << "W" << info[i].value1; break;
				case 'T':
					if ( info[i].kind == Kind::Student )
						cout << "T" << info[i].value1 << "," << info[i].value2 << "," << info[i].c;
					else
						cout << "T" << info[i].value1 << "," << info[i].value2; break;
				case 'F':
					cout << "F"; break;
				case 'L':
					if ( info[i].kind == Kind::WATCardOfficeCourier ||
							( info[i].kind == Kind::NameServer && info[i].isTimer == false ) )
							cout << "L" << info[i].value1;
					else cout << "L"; break;
				case 'E':
					if ( info[i].kind == Kind::Student )
						cout << "E" << info[i].value1;
					else cout << "E" << info[i].value1 << "," << info[i].value2;
					break;
				case 't':
					if ( info[i].kind == Kind::TrainStop )
						cout << "t";
					else if ( info[i].kind == Kind::WATCardOfficeCourier )
						cout << "t" << info[i].value1 << "," << info[i].value2;
					else cout << "t" << info[i].value1; 
					break;
				case 'R':
					cout << "R" << info[i].value1; break;
				case 'c': cout << "c"; break;
				case 'e':
					if ( info[i].kind == Kind::Conductor ) cout << "e" << info[i].value1;
					else cout << "e"; break;
				case 'f': cout << "f"; break;
			} // switch

			info[i].isFlushed = true;
			if ( i != index) cout << '\t';
		} // for
		cout << endl;
	}
};

Printer::Printer( unsigned int numStudents, unsigned int numTrains, unsigned int numStops, unsigned int numCouriers )
{
	pimpl = new PImpl { numStudents, numTrains, numStops, numCouriers };
}

Printer::~Printer() {
	delete pimpl;
}

void Printer::print( Kind kind, char state ) {
	if ( !p.info[kind].isFlushed ) p.flush();
	p.info[kind].kind = kind;
	p.info[kind].state = state;
	p.info[kind].isFlushed = false;

	if ( kind == Kind::NameServer && state == 'L' ) p.info[kind].isTimer = true;
}

void Printer::print( Kind kind, char state, unsigned int value1 ) {
	if ( !p.info[kind].isFlushed ) p.flush();
	p.info[kind].kind = kind;
	p.info[kind].state = state;
	p.info[kind].value1 = value1;
	p.info[kind].isFlushed = false;

	if ( kind == Kind::NameServer && state == 'L' ) p.info[kind].isTimer = false;
}

void Printer::print( Kind kind, char state, unsigned int value1, unsigned int value2 ) {
	if ( !p.info[kind].isFlushed ) p.flush();
	p.info[kind].kind = kind;
	p.info[kind].state = state;
	p.info[kind].value1 = value1;
	p.info[kind].value2 = value2;
	p.info[kind].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, unsigned int value1 ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].value1 = value1;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2 ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].value1 = value1;
	p.info[kind + offset + lid].value2 = value2;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, unsigned int oid, unsigned int value1, unsigned int value2 ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].oid = oid;
	p.info[kind + offset + lid].value1 = value1;
	p.info[kind + offset + lid].value2 = value2;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, char c ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].c = c;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, unsigned int value1, char c ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].value1 = value1;
	p.info[kind + offset + lid].c = c;
	p.info[kind + offset + lid].isFlushed = false;
}

void Printer::print( Kind kind, unsigned int lid, char state, unsigned int value1, unsigned int value2, char c ) {
	int offset = p.getOffset( kind );
	if ( !p.info[kind + offset + lid].isFlushed ) p.flush();
	p.info[kind + offset + lid].kind = kind;
	p.info[kind + offset + lid].state = state;
	p.info[kind + offset + lid].value1 = value1;
	p.info[kind + offset + lid].value2 = value2;
	p.info[kind + offset + lid].c = c;
	p.info[kind + offset + lid].isFlushed = false;
}
