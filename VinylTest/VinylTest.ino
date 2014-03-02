#include "Vinyl.h"

Vinyl vinyl(A1);

void setup(){
}

void loop(){
	if (vinyl.hitLine()){
		Serial.println("LINE");
	}
	else{
		Serial.println("NO LINE");
	}
	delay(500);
}