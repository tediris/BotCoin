// BotCoinTesting.ino
#include "Timer.h"
//#include "FrequencyDetector.h"

#define LEFT A5
#define MIDDLE A6
#define RIGHT A7

#define BEACON_CENTER 0
#define BEACON_LEFT 1
#define BEACON_RIGHT 2
#define NO_BEACON 3

#define MOTOR_A_ENABLE 12
#define MOTOR_B_ENABLE 3
#define MOTOR_A_DIR 13
#define MOTOR_B_DIR 11

#define NONE -1
#define DRIVING_FORWARD_DEBOUNCE_TIME 500

#define BASE_SPEED 180
#define TURN_TIME 300
#define SCAN_TIME 600

#define LOW_FREQ 1176
#define HIGH_FREQ 333

#define NO_FREQUENCY 0
#define SERVER_FREQUENCY 1
#define EXCHANGE_FREQUENCY 2

#define INTERRUPT_PIN 0

volatile bool firstWave = false;
volatile unsigned long firstEdgeTime = 0;
volatile unsigned long secondEdgeTime = 0;
volatile bool frequencyDataReady = false;

int motorADir = LOW;
int motorBDir = HIGH;

bool rotating = false;
bool scanning = false;
bool drivingForward = false;


Timer turnTimer(TURN_TIME);
Timer scanTimer(SCAN_TIME);
Timer forwardDebounceTimer(DRIVING_FORWARD_DEBOUNCE_TIME);

//FrequencyDetector freqDetector(0);

void setup() {
	Serial.begin(9600);
	//initBeaconDetectors();
	//initMotors();
	//drive(0, 0);
	//drive(-255, -255);
	//delay(2000); //allow things to initialize
	//prepFrequencyData();
}

void loop() {
	if (Serial.available()) {
		while (Serial.available()) Serial.read();
		Serial.println("Getting Data");
		prepFrequencyData();
		while (!frequencyDataReady);

		int freq = getFrequency();
		if (freq == EXCHANGE_FREQUENCY) {
			Serial.println("Exchange");
		} else if (freq == SERVER_FREQUENCY) {
			Serial.println("Server");
		} else {
			Serial.println("None");
		}
	}
}



int getFrequency() {
	frequencyDataReady = false;

	unsigned long difference = secondEdgeTime - firstEdgeTime;
    Serial.println(difference);
    if (difference > 800 && difference < 1300) {
    	return SERVER_FREQUENCY;
    } else if (difference > 200 && difference < 500) {
    	return EXCHANGE_FREQUENCY;
    } else {
    	return NO_FREQUENCY;
    }
}

void cancelFrequencyRequest() {

}

void prepFrequencyData() {
	firstEdgeTime = 0;
	secondEdgeTime = 0;
	firstWave = true;
	frequencyDataReady = false;
	attachInterrupt(INTERRUPT_PIN, getFrequencyData, RISING); 
}

void getFrequencyData() {
	if (firstWave) {
 		firstWave = false;
   		firstEdgeTime = micros();
	} else { //2nd edge
   		secondEdgeTime = micros();
   		frequencyDataReady = true;
   		detachInterrupt(INTERRUPT_PIN);
 	}
}



/* function: debugBeacon()
 * -----------------------
 * Prints to Serial current beacon position relative to triple sensor
 */

void debugBeacon() {
	int beaconPosition = getBeaconPosition();

	if (beaconPosition == BEACON_CENTER) {	
		Serial.println("CENTER CLOSE");
	} else if (beaconPosition == BEACON_LEFT) {
		Serial.println("BEACON LEFT");
	} else if (beaconPosition == BEACON_RIGHT) {
		Serial.println("BEACON RIGHT");
	} else if (beaconPosition == NO_BEACON) {
		Serial.println("NO BEACON");
	}
	delay(1000);
}


/* function: followBeacon()
 * ------------------------
 * Spins around looking for a beacon, and once found,
 * drives towards it and hits the structure with the
 * beacon.
 */

void followBeacon() {
	
	if (rotating) {
		if (turnTimer.isExpired()) {
			rotating = false;
			scanning = true;
			drive(0, 0);
			scanTimer.start();
		}
		return;
	}

	if (scanning) {
		if (scanTimer.isExpired()) {
			scanning = false;
		}
		return;
	}
	

	int beaconPosition = getBeaconPosition();
	if (beaconPosition == BEACON_CENTER) {
		drivingForward = true;
		forwardDebounceTimer.start();

		driveForward(BASE_SPEED);
	} else if (beaconPosition == BEACON_LEFT) {
		drivingForward = true;
		forwardDebounceTimer.start();

		drive(BASE_SPEED*0.9, BASE_SPEED*1.1);
	} else if (beaconPosition == BEACON_RIGHT) {
		forwardDebounceTimer.start();
		drivingForward = true;
		drive(BASE_SPEED*1.1, BASE_SPEED*0.9);
	} else if (beaconPosition == NO_BEACON) {
		if (drivingForward) {
			if (forwardDebounceTimer.isExpired()) drivingForward = false;
		} else {
			rotateLeft(BASE_SPEED*0.8);
			rotating = true;
			turnTimer.start();
		}
	}
}

void rotateLeft(int speed) {
	drive(-speed, speed);
}

void rotateRight(int speed) {
	drive(speed, -speed);
}

void driveForward(int speed) {
	drive(speed, speed);
}

void drive(int leftSpeed, int rightSpeed) {
	if (leftSpeed > 0) {
		
		digitalWrite(MOTOR_A_DIR, LOW);
			
	} else if (leftSpeed < 0) {
		leftSpeed = leftSpeed*-1;
		digitalWrite(MOTOR_A_DIR, HIGH);
			
	} else {
		digitalWrite(MOTOR_A_DIR, HIGH);
	}

	if (rightSpeed > 0) {
		
		digitalWrite(MOTOR_B_DIR, HIGH);
			
	} else if (rightSpeed < 0) {
		rightSpeed = rightSpeed*-1;
		
		digitalWrite(MOTOR_B_DIR, LOW);
			
	} else {
		digitalWrite(MOTOR_B_DIR, LOW);
	}

	analogWrite(MOTOR_A_ENABLE, leftSpeed);
	analogWrite(MOTOR_B_ENABLE, rightSpeed);
}

void switchDirection() {
	if (motorADir == LOW) {
		digitalWrite(MOTOR_A_DIR, HIGH);
		motorADir = HIGH;
	} else {
		digitalWrite(MOTOR_A_DIR, LOW);
		motorADir = LOW;
	}

	if (motorBDir == LOW) {
		digitalWrite(MOTOR_B_DIR, HIGH);
		motorBDir = HIGH;
	} else {
		digitalWrite(MOTOR_B_DIR, LOW);
		motorBDir = LOW;
	}
}

/* Motor Debug Functions */

void writeSpeed(int speed) {
	analogWrite(MOTOR_A_ENABLE, speed);
	analogWrite(MOTOR_B_ENABLE, speed);
} 

int getBeaconPosition() {
	int leftRead = analogRead(LEFT);
	int middleRead = analogRead(MIDDLE);
	int rightRead = analogRead(RIGHT); 
 
	if ((leftRead > 400 && middleRead > 400 && rightRead > 400) || (abs(leftRead - rightRead) < 50 && leftRead > 100)) {
		return BEACON_CENTER;
	} else if (middleRead > 200 && middleRead > leftRead) {
		return BEACON_RIGHT;
	} else if (middleRead > 200 && middleRead > rightRead) {
		return BEACON_LEFT;
	} else {
  		return NO_BEACON;
 	}
}

/* Initialization Code */

void initBeaconDetectors() {
	pinMode(LEFT, INPUT);
	pinMode(MIDDLE, INPUT);
	pinMode(RIGHT, INPUT);
}

void initMotors() {
	pinMode(MOTOR_A_ENABLE, OUTPUT);
	pinMode(MOTOR_B_ENABLE, OUTPUT);
	pinMode(MOTOR_A_DIR, OUTPUT);
	pinMode(MOTOR_B_DIR, OUTPUT);

	analogWrite(MOTOR_B_ENABLE, 0);
	analogWrite(MOTOR_A_ENABLE, 0);
	digitalWrite(MOTOR_A_DIR, HIGH);
	digitalWrite(MOTOR_B_DIR, LOW);

}