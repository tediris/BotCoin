// BotCoinTesting.ino
#include "Timer.h"
#include "Vinyl.h"
//#include "FrequencyDetector.h"

#define LEFT A5
#define MIDDLE A6
#define RIGHT A7
#define MIDDLE_BACK A10
#define MIDDLE_LEFT A9

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

#define BASE_SPEED 240
#define TURN_TIME 250
#define SCAN_TIME 600
#define DEBOUNCE_SPIN_TIME 800
#define SENSOR_RESET_TIME 500
#define REVERSE_TIME 500
#define REST_MOTOR_TIME 500
#define RESET_MOTOR_TIME 500

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
Timer debounceSpin(DEBOUNCE_SPIN_TIME);
Timer forwardDebounceTimer(DRIVING_FORWARD_DEBOUNCE_TIME);
Timer sensorResetTimer(SENSOR_RESET_TIME);
Timer reverseTimer(REVERSE_TIME);
Timer restMotorTimer(REST_MOTOR_TIME);
Timer resetMotorTimer(RESET_MOTOR_TIME);

Vinyl vinyl(A0);

#define ARCING_LEFT 0
#define ARCING_RIGHT 1
#define NO_ARC 2

int beaconFollowingState = -1;
bool lostBeacon = false;
#define LOST_BEACON_TIME 1500
Timer lostBeaconTimer(LOST_BEACON_TIME);

Timer lostBeaconScan(600);
Timer lostBeaconTurn(250);

/* States */
#define SPINNING_STRONG 0
#define SPINNING_WEAK 5
#define MOVING_FORWARD 1
#define SEEKING_STRONG 2
#define SEEKING_WEAK 3
#define MOVING_TO_VINYL_STRONG 4
#define DEBOUNCE_SPIN 6
#define CALMING_SENSORS 7
#define RESTING_MOTORS 9
#define REVERSING 8
#define DEAD 69

#define RESET_MOTORS 11
#define RELOCATING_BEACON_SCAN 12
#define RELOCATING_BEACON_TURN 13
#define MOVING_TO_SERVER 14

int dogeState = SEEKING_STRONG;

//FrequencyDetector freqDetector(0);

void setup() {
	Serial.begin(9600);
	initBeaconDetectors();
	initMotors();
	drive(0, 0);
	//drive(-255, -255);
	delay(2000); //allow things to initialize
	//prepFrequencyData();
	scanTimer.start();
}

bool hitVinyl() {
	return analogRead(A0) < 750;
}

void loop() {
	brickBeater();
	//driveForward(BASE_SPEED);
	//testVinyl();
	//testBeacon();
}

void testBeacon() {
	int avg = getAverageBeaconValue();
	Serial.println(avg);
	delay(100);
}

void testVinyl() {
	if (hitVinyl()) {
		Serial.println("Vinyl detected");
	}
}

void brickBeater() {

	int beaconReading = 0;

	switch(dogeState) {
		

		case SPINNING_STRONG:
			Serial.println("SPINNING_STRONG");

			rotateRight(BASE_SPEED*0.8);
			if (turnTimer.isExpired()) {
				dogeState = SEEKING_STRONG;
				drive(0, 0);
				scanTimer.start();
			}
			break;

		case SPINNING_WEAK:
			Serial.println("SPINNING_WEAK");

			rotateRight(BASE_SPEED*0.8);
			if (turnTimer.isExpired()) {
				dogeState = SEEKING_WEAK;
				drive(0, 0);
				scanTimer.start();
			}
			break;

	    case SEEKING_STRONG:
	    	Serial.println("SEEKING_STRONG");
	    	drive(0, 0);
	      	beaconReading = analogRead(MIDDLE);
	      	if (beaconReading > 130) {

	      		dogeState = RESET_MOTORS;
	      		resetMotorTimer.start();
	      		break;

	      		} else if (scanTimer.isExpired()) {
	      			dogeState = SPINNING_STRONG;
	      			turnTimer.start();
	      		}
	    	break;

	    case RESET_MOTORS:
	    	Serial.println("RESET_MOTORS");
	    	drive(0, 0);
	    	if (resetMotorTimer.isExpired()) {
	    		dogeState = MOVING_TO_VINYL_STRONG;
	    	}
	    	break;

	    case SEEKING_WEAK:
	    	Serial.println("SEEKING_WEAK");
	    	drive(0, 0);
	    	beaconReading = getAverageBeaconValue();
	      	if (beaconReading > 50 && beaconReading < 150 && analogRead(MIDDLE) < 150 && analogRead(MIDDLE) > 50) {
	      		dogeState = MOVING_TO_SERVER;
	      		} else if (scanTimer.isExpired()) {
	      			dogeState = SPINNING_WEAK;
	      			turnTimer.start();
	      		}
	    	break;

	    case MOVING_TO_SERVER:
	    	driveToServer();

	    	break;

	    case MOVING_TO_VINYL_STRONG:
	    	Serial.println("MOVING_TO_VINYL_STRONG");
	    	//driveForward(BASE_SPEED*0.8);
	    	driveToBeacon();
	    	if (hitVinyl()) {
	    		dogeState = REVERSING;
	    		drive(0, 0);
	    		reverseTimer.start();
	    	}
	    	break;

	    case DEBOUNCE_SPIN:
	    	Serial.println("DEBOUNCE_SPIN");
	    	rotateRight(BASE_SPEED*0.8);
	    	if (debounceSpin.isExpired()) {
	    		dogeState = CALMING_SENSORS;
	    		drive(0, 0);
	    		sensorResetTimer.start();
	    	}
	    	break;
	      // do something
	     case CALMING_SENSORS:
	     Serial.println("CALMING_SENSORS");
	     	if (sensorResetTimer.isExpired()) {
	     		dogeState = SEEKING_WEAK;
	     		scanTimer.start();
	     	}
	     	break;

	     case REVERSING:
	     	Serial.println("REVERSING");
	     	drive(BASE_SPEED*-0.8, BASE_SPEED*-0.8);
	     	if (reverseTimer.isExpired()) {
	     		dogeState = RESTING_MOTORS;
	     		restMotorTimer.start();
	     	}
	     	break;

	     case RESTING_MOTORS:
	     	Serial.println("RESTING_MOTORS");
	     	drive(0, 0);
	     	if (restMotorTimer.isExpired()) {
	     		dogeState = DEBOUNCE_SPIN;
	     		debounceSpin.start();
	     	}
	     	break;

	     case RELOCATING_BEACON_SCAN:
	     	//Serial.println
	     	drive(0, 0);
	    	beaconReading = getAverageBeaconValue();
	      	if (beaconReading > 50 && beaconReading < 150 && analogRead(MIDDLE) < 150 && analogRead(MIDDLE) > 50) {
	      		dogeState = DEAD;
	      		} else if (lostBeaconScan.isExpired()) {
	      			dogeState = RELOCATING_BEACON_TURN;
	      			lostBeaconTurn.start();
	      		}
	    	break;

	    case RELOCATING_BEACON_TURN:

	    	if (beaconFollowingState == ARCING_RIGHT) rotateRight(BASE_SPEED*0.8);
	    	else if (beaconFollowingState == ARCING_LEFT) rotateLeft(BASE_SPEED*0.8);
	    	else rotateRight(BASE_SPEED*0.8);

			if (lostBeaconTurn.isExpired()) {
				dogeState = RELOCATING_BEACON_SCAN;
				drive(0, 0);
				lostBeaconScan.start();
			}
			break;

	     case DEAD:
	     	drive(0, 0);
	     	//Serial.println("DEAD");
	     	//driveToBeacon();
	     	break;

	}
}

void driveToServer() {
	int beaconPosition = getBeaconPosition();
	if (beaconPosition == BEACON_CENTER) {
		driveForward(BASE_SPEED);
		beaconFollowingState = NO_ARC;
		lostBeacon = false;
	} else if (beaconPosition == BEACON_LEFT) {
		drive(BASE_SPEED*0.95, BASE_SPEED*1.05);
		beaconFollowingState = ARCING_LEFT;
		lostBeacon = false;
	} else if (beaconPosition == BEACON_RIGHT) {
		drive(BASE_SPEED*1.05, BASE_SPEED*0.95);
		beaconFollowingState = ARCING_RIGHT;
		lostBeacon = false;
	} else if (beaconPosition == NO_BEACON) {
		//maybe have a state change? to relocate beacon?
		if (lostBeacon) {
			if (lostBeaconTimer.isExpired()) {
				dogeState = RELOCATING_BEACON_SCAN;
				lostBeaconScan.start();
				//dogeState = SEEKING_WEAK;
				lostBeacon = false;
			}
		} else {
			lostBeacon = true;
			lostBeaconTimer.start();
		}
		driveForward(BASE_SPEED);
	}	
}

void driveToBeacon() {
	int beaconPosition = getBeaconPosition();
	if (beaconPosition == BEACON_CENTER) {
		driveForward(BASE_SPEED);
	} else if (beaconPosition == BEACON_LEFT) {
		drive(BASE_SPEED*0.95, BASE_SPEED*1.05);
	} else if (beaconPosition == BEACON_RIGHT) {
		drive(BASE_SPEED*1.05, BASE_SPEED*0.95);
	} else if (beaconPosition == NO_BEACON) {
		//maybe have a state change? to relocate beacon?
		driveForward(BASE_SPEED);
	}
}

void frequencyCheckLoop() {
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
    } else if (difference > 150 && difference < 500) {
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

int getAverageBeaconValue() {
	int leftRead = analogRead(LEFT);
	int middleRead = analogRead(MIDDLE);
	int rightRead = analogRead(RIGHT); 

	unsigned int avg = leftRead + rightRead + middleRead;
	avg = avg/3;

	Serial.print("Average Beacon Value: ");
	Serial.println(avg);

	return avg;
}

int getBeaconPosition() {
	int leftRead = analogRead(LEFT);
	int middleRead = analogRead(MIDDLE);
	int rightRead = analogRead(RIGHT); 

	unsigned int avg = leftRead + rightRead + middleRead;
	avg = avg/3;

	Serial.print("Average: ");
	Serial.println(avg);
 
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