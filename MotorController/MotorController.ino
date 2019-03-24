// CMSC838J, Spring 2019
// Final Project
// Members: Debjani Saha, Andrew Fichman
//
// High-level description of behavior:
// 'r\n' -> right motor turns, pauses, turns opposite direction
// 'l\n' -> left motor turns, pauses, turns opposite direction
//  _    -> No-op.

////////////////////////////////////////
// Configs/constants
////////////////////////////////////////

// Motor configs.
struct MotorConfig {
  unsigned int stepPin, dirPin;
};
const MotorConfig rConfig={.stepPin=5, .dirPin=4}; // RIGHT
const MotorConfig lConfig={.stepPin=0, .dirPin=2}; // LEFT

// Motor delays for the OFF/ON modes.
const unsigned int DELAY_MICRO_LOW=1000;
const unsigned int DELAY_MICRO_HIGH=1000;

// Directions.
const unsigned int LEFT = 0;
const unsigned int RIGHT = 1;

// Time to spin in a given direction.
const unsigned int SPIN_TIME_MICRO = 1e6/4;

// Time to pause during a change in direction.
const unsigned int PAUSE_TIME_MICRO = 1e6/4;

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

void turnOff(MotorConfig mc) {
  digitalWrite(mc.stepPin,LOW);
}

void spin(MotorConfig mc, unsigned int dir) {
  const int numIter = SPIN_TIME_MICRO/(DELAY_MICRO_LOW+DELAY_MICRO_HIGH); 
  for(int i=0; i<numIter; i++) {
    // Set direction.
    digitalWrite(mc.dirPin,dir);

    // Run motor.
    digitalWrite(mc.stepPin,HIGH);
    delayMicroseconds(DELAY_MICRO_HIGH);

    // Rest motor.
    digitalWrite(mc.stepPin,LOW);
    delayMicroseconds(DELAY_MICRO_LOW);

    // Let background activities run.
    yield();
  }
}

////////////////////////////////////////
// Initialization
////////////////////////////////////////

void setup() {
  pinMode(rConfig.stepPin,OUTPUT);
  pinMode(rConfig.dirPin,OUTPUT);
  pinMode(lConfig.stepPin,OUTPUT);
  pinMode(lConfig.dirPin,OUTPUT);
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
  const MotorConfig mc = cmd=='l' ? lConfig : rConfig;

  // Spin initial direction.
  spin(mc,dir);
  turnOff(mc);

  // Delay.
  delayMicroseconds(PAUSE_TIME_MICRO);

  // Spin opposite initial direction.
  spin(mc,!dir);
  turnOff(mc);

  // Clear buffer, since we don't want to queue up commands.
  // Rather, we want to respond in real time to one action
  // at a time.
  clearInputBuffer();
}
