#include "Drive.h"

Drive drive(240, 13, 12, 11, 3);

void setup(){
	drive.init(LOW, HIGH);
}

void loop(){
	drive.stop();
}