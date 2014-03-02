// TimerTest.ino
#include "Timer.h"


int counter;
Timer timer1(2000);

void setup() {
	timer1.start();
	Serial.begin(9600);
	counter = 0;
}

void loop() {
	if (timer1.isExpired()){
		counter++;
		Serial.print("Timer1 expired #");
		Serial.println(counter);
		timer1((unsigned long)2000);
		timer1.start();
	}
}

