#include "Drive.h"

Drive drive(240, 13, 12, 11, 3);

void setup(){
	drive.init(HIGH, LOW);
}

void loop(){
	drive.driveForward(10);
	delay(2000);
	drive.stop();
	delay(1000);
	drive.rotateRight(8);
	delay(1000);
	drive.stop();
	delay(1000);
	drive.rotateLeft(8);
	delay(1000);
	drive.driveBackward(10);
	delay(2000);
	drive.stop();
	delay(1000);
}