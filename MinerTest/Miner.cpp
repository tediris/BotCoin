#include "Arduino.h"
#include "Miner.h"

Miner::Miner(int pin){
	_mpin = pin;
}

void Miner::init(int min, int max){
	_miner.attach(_pin);
	_min = min;
	_max = max;
}

void Miner::extend(){
	_miner.write(_max);
}

void Miner::retract(){
	_miner.write(_min);
}