// TimerTest.ino
#include "Timer.h"

int counter;
Timer timer1(5000);

bool done = false;

void setup() {
	timer1.start();
	Serial.begin(9600);
	counter = 0;
	Serial.print("CURRENT TIME: ");
	Serial.println(millis());
}

void loop() {
	if (timer1.isExpired() && !done){
		counter++;
		Serial.println("Timer1 expired");
		//Serial.println(counter);
		timer1.set(3000);
		timer1.start();
		Serial.print("FINAL TIME: ");
		Serial.println(millis());
	}
}

