#ifndef Vinyl_h
#define Vinyl_h

#include "Arduino.h"

class Vinyl{
public:
	Vinyl(int pin);
	bool hitLine();
private:
	int _pin;
	int _initialRead;
};

#endif