#include "Vinyl.h"

Vinyl vinyl(A0);

void setup(){
	Serial.begin(9600);
}

void loop(){
	if (vinyl.hitLine()){
		Serial.println("LINE");
	}
}