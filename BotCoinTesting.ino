// BotCoinTesting.ino

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
#define DRIVING_FORWARD_DEBOUNCE 5

int motorADir = LOW;
int motorBDir = HIGH;

bool rotating = false;
bool drivingForward = false;
int forwardCounter = 0;

void setup() {
	//Serial.begin(9600);
	//initBeaconDetectors();
	initMotors();
	drive(0, 0);
	//drive(-255, -255);
	delay(2000);
}

void loop() {
	followBeacon();
	//debugBeacon();
}

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

void followBeacon() {
	if (rotating) {
		delay(500);
		drive(0, 0);
		delay(500);
		rotating = false;
	}
	int beaconPosition = getBeaconPosition();
	if (beaconPosition == BEACON_CENTER) {
		driveForward(150);
	} else if (beaconPosition == BEACON_LEFT) {
		forwardCounter = 0;
		drivingForward = true;
		drive(140, 160);
	} else if (beaconPosition == BEACON_RIGHT) {
		forwardCounter = 0;
		drivingForward = true;
		drive(160, 140);
	} else if (beaconPosition == NO_BEACON) {
		if (drivingForward) {
			forwardCounter++;
			if (forwardCounter == DRIVING_FORWARD_DEBOUNCE) drivingForward = false;
		} else {
			rotateLeft(130);
			rotating = true;
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
	forwardCounter = 0;
	drivingForward = true;
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

/*
void drive(int leftSpeed, int rightSpeed) {
	bool scheduleDelay = false;

	if (leftSpeed > 0) {
		if (motorADir == LOW) { 
			digitalWrite(MOTOR_A_DIR, HIGH);
			scheduleDelay = true;
			motorADir = HIGH;
		}
	} else if (leftSpeed < 0) {
		leftSpeed = leftSpeed*-1;
		if (motorADir == HIGH) { 
			digitalWrite(MOTOR_A_DIR, LOW);
			scheduleDelay = true;
			motorADir = LOW;
		}
	} else {
		digitalWrite(MOTOR_A_DIR, LOW);
	}

	if (rightSpeed > 0) {
		if (motorBDir == HIGH) {
			digitalWrite(MOTOR_B_DIR, LOW);
			scheduleDelay = true;
			motorBDir = LOW;
		}
	} else if (rightSpeed < 0) {
		rightSpeed = rightSpeed*-1;
		if (motorBDir == LOW) {
			digitalWrite(MOTOR_B_DIR, HIGH);
			scheduleDelay = true;
			motorBDir = HIGH;
		}
	} else {
		digitalWrite(MOTOR_B_DIR, HIGH);
	}

	if (scheduleDelay) {
		delay(500);
	}

	analogWrite(MOTOR_A_ENABLE, leftSpeed);
	analogWrite(MOTOR_B_ENABLE, rightSpeed);
}

*/

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