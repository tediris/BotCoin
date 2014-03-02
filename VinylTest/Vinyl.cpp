#include "Arduino.h"
#include "Vinyl.h"

#define BUFFER 250

Vinyl::Vinyl(int pin){
	pinMode(pin, INPUT);
	_initialRead = analogRead(pin);
	_pin = pin;
}

bool Vinyl::hitLine(){
	int curRead = analogRead(_pin);
	if (curRead < _initialRead - BUFFER){
		return true;
	}
	_initialRead = curRead;
	return false;
}