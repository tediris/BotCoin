#ifndef Miner_h
#define Miner_h

#include "Arduino.h"
#include "Servo.h"

class Miner {
public:
	Miner(int pin);
	void init(int min, int max);
	void extend();
	void retract();

private:
	Servo _miner;
	int _pin;
	int _min;
	int _max;
};

#endif