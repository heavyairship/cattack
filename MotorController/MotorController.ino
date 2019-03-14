// CMSC838J, Spring 2019
// Final Project
//
// Moves two motors with serial input.
// 0 -> ON.
// anything else -> OFF.


////////////////////////////////////////
// Configs 
////////////////////////////////////////

// Motor configs. To run motors in opposite directions,
// change the .dir attribute to LOW on one of them.
struct MotorConfig {
  int stepPin, dirPin, dir;
};
const MotorConfig motorConfig1={.stepPin=5, .dirPin=4, .dir=LOW};
const MotorConfig motorConfig2={.stepPin=0, .dirPin=2, .dir=HIGH};

// Motor delays for the ON mode.
const int delayMicroLow=100;
const int delayMicroHigh=1000;


////////////////////////////////////////
// State
////////////////////////////////////////

// Current state of motors.
enum MotorState {ON, OFF};
MotorState motorState=OFF;


////////////////////////////////////////
// Motor functions
////////////////////////////////////////

void spin(MotorConfig mc) {
  // Spins motor using given configuration.
  digitalWrite(mc.dirPin,mc.dir);
  digitalWrite(mc.stepPin,HIGH);
  delayMicroseconds(delayMicroHigh);
  digitalWrite(mc.stepPin,LOW);
  delayMicroseconds(delayMicroLow);
}

void turnOff(MotorConfig mc) {
  digitalWrite(mc.stepPin,LOW);
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
  // Read input.
  if(Serial.available()>0){  
    motorState = (MotorState)Serial.parseInt();
    Serial.read();
  }
  switch(motorState) {
    case ON: 
      // Spin motors.
      for(int i=0; i<200; i++) {
        spin(motorConfig1);
        spin(motorConfig2);
        yield();
      }
      break;
    case OFF:
      // Turn motors off.
      turnOff(motorConfig1);
      turnOff(motorConfig2);
      break;
    default: 
      Serial.write("Error: unhandled case");
  }
}
