// CMSC838J, Spring 2019
// Final Project
//
// High-level description of behavior:
// 'l\n' -> both motors turn left for 1 second, pause for 1 second, turn right for 1 second, then clear input buffer
// 'r\n' -> both motors turn right for 1 second, pause for 1 second, turn left for 1 second, then clear input buffer
//  _    -> No-op.

////////////////////////////////////////
// Configs/constants
////////////////////////////////////////

// Motor configs.
struct MotorConfig {
  unsigned int stepPin, dirPin;
};
const MotorConfig motorConfig1={.stepPin=5, .dirPin=4};
const MotorConfig motorConfig2={.stepPin=0, .dirPin=2};

// Motor delays for the ON mode.
const unsigned int DELAY_MICRO_LOW=100;
const unsigned int DELAY_MICRO_HIGH=1000;

// Directions
const unsigned int LEFT = 0;
const unsigned int RIGHT = 0;

// Time to spin in a given direction
const unsigned int SPIN_TIME_MICRO = 1e6;
const unsigned int PAUSE_TIME_MICRO = 1e6/2;

////////////////////////////////////////
// Util functions
////////////////////////////////////////

void clearInputBuffer() {
  while(Serial.available()) {
    Serial.read();
    // FixMe: maybe yield every so often, if this becomes an issue.
  }
}

////////////////////////////////////////
// Motor functions
////////////////////////////////////////

void turnOffBoth() {
  digitalWrite(motorConfig1.stepPin,LOW);
  digitalWrite(motorConfig2.stepPin,LOW);
}

void spinBoth(unsigned int dir) {
  // Spins both motors in given direction.
  const int numIter = SPIN_TIME_MICRO/(DELAY_MICRO_LOW+DELAY_MICRO_HIGH);
  for(int i=0; i<numIter; i++) {
    // Set direction.
    digitalWrite(motorConfig1.dirPin,dir);
    digitalWrite(motorConfig2.dirPin,dir);

    // Run motors
    digitalWrite(motorConfig1.stepPin,HIGH);
    digitalWrite(motorConfig2.stepPin,HIGH);
    delayMicroseconds(DELAY_MICRO_HIGH);

    // Rest motors
    digitalWrite(motorConfig1.stepPin,LOW);
    digitalWrite(motorConfig2.stepPin,LOW);
    delayMicroseconds(DELAY_MICRO_LOW);

    // Let background activities run.
    yield();
  }
}

////////////////////////////////////////
// Initialization
////////////////////////////////////////

void setup() {
  pinMode(motorConfig1.stepPin,OUTPUT);
  pinMode(motorConfig1.dirPin,OUTPUT);
  pinMode(motorConfig2.stepPin,OUTPUT);
  pinMode(motorConfig2.dirPin,OUTPUT);
  Serial.begin(9600);
}

////////////////////////////////////////
// Main loop
////////////////////////////////////////

void loop() {
  if(Serial.available()<2){  
    // Nothing to do.
    return;
  }

  // Read input.
  const byte cmd = Serial.read();
  const byte newline = Serial.read();
  if(!((cmd=='l' || cmd=='r') && newline=='\n')) {
    Serial.write('Error: invalid input\n');
    return;
  } 

  // Determine initial direction.
  const unsigned int dir = cmd=='l' ? LEFT : RIGHT;

  // Spin initial direction
  spinBoth(dir);
  turnOffBoth();

  // Delay.
  delayMicroseconds(PAUSE_TIME_MICRO);

  // Spin opposite initial direction.
  spinBoth(!dir);
  turnOffBoth();

  // Clear buffer, since we don't want to queue up commands.
  // Rather, we want to respond in real time to one action
  // at a time.
  clearInputBuffer();
}
