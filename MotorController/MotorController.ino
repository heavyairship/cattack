// CMSC838J, Spring 2019
// Final Project
// Members: Debjani Saha, Andrew Fichman
//
// High-level description of behavior:
// left photometer detects motion -> right motor turns, pauses, turns opposite direction
// right photometer detects motion -> left motor turns, pauses, turns opposite direction

////////////////////////////////////////
// Configs/constants
////////////////////////////////////////

// Motor configs.
struct MotorConfig {
  unsigned int stepPin, dirPin;
};
const MotorConfig RCONFIG={.stepPin=D1, .dirPin=D2}; // RIGHT
const MotorConfig LCONFIG={.stepPin=D3, .dirPin=D4}; // LEFT

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

// Input photo analog pin
const unsigned int PHOTO = A0;

// Fraction of photo baseline to use as threshold.
const float THRESHOLD_FACTOR = 0.07;

// Multiplexer constants
const unsigned int MUX_A = D8;
const unsigned int MUX_B = D7;
const unsigned int MUX_C = D6;

// Led constants
const unsigned int LED = D5;
const unsigned int LED_TIMEOUT = 500;

////////////////////////////////////////
// Utility functions
////////////////////////////////////////

void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
}

////////////////////////////////////////
// Photo state
////////////////////////////////////////

int lPhotoPrev = -1;
int rPhotoPrev = -1;

////////////////////////////////////////
// LED state
////////////////////////////////////////

unsigned int ledState = LOW;
unsigned int ledStartTime = 0;

////////////////////////////////////////
// Motor state
////////////////////////////////////////

bool movedPrevIter = false;

////////////////////////////////////////
// Photo functions
////////////////////////////////////////

void selectPhotoInput(unsigned int side) {
  if(side == LEFT) {
    changeMux(LOW, LOW, LOW);
  } else {
    changeMux(LOW, LOW, HIGH);
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
  Serial.begin(9600);
  delay(1000); // Prevents omission of early messages from output console.
  pinMode(RCONFIG.stepPin,OUTPUT);
  pinMode(RCONFIG.dirPin,OUTPUT);
  pinMode(LCONFIG.stepPin,OUTPUT);
  pinMode(LCONFIG.dirPin,OUTPUT);
  pinMode(MUX_A,OUTPUT);
  pinMode(MUX_B,OUTPUT);
  pinMode(MUX_C,OUTPUT);
  pinMode(LED,OUTPUT);
  ledStartTime = millis();
}

////////////////////////////////////////
// Main loop
////////////////////////////////////////

void loop() {
  // Blink LED
  if((millis()-ledStartTime)>LED_TIMEOUT) {
    ledState = !ledState;
    digitalWrite(LED,ledState);
    ledStartTime = millis();
  }  

  // Read current photo values
  selectPhotoInput(LEFT);
  const int lPhotoIntensity = analogRead(PHOTO);
  selectPhotoInput(RIGHT);
  const int rPhotoIntensity = analogRead(PHOTO);  
  
  // Read new photo values and compute % change.
  const float lPhotoChange = lPhotoPrev == -1 ? 0 : abs(lPhotoIntensity-lPhotoPrev)/(float)lPhotoPrev;
  const float rPhotoChange = rPhotoPrev == -1 ? 0 : abs(rPhotoIntensity-rPhotoPrev)/(float)rPhotoPrev;
  lPhotoPrev = lPhotoIntensity;
  rPhotoPrev = rPhotoIntensity;
 
  if(movedPrevIter) {
    // Never move two iterations in a row. 
    // This is a bit of a hack to avoid
    // double moves.
    movedPrevIter = false;
    return;
  }

  // Determine motor direction
  unsigned int dir;
  MotorConfig mc;
  if(lPhotoChange>THRESHOLD_FACTOR && !(rPhotoChange>THRESHOLD_FACTOR)) {
    // Motion on left only -> move direction right
    dir = RIGHT;
    mc = RCONFIG;
  } else if(rPhotoChange>THRESHOLD_FACTOR && !(lPhotoChange>THRESHOLD_FACTOR)) {
    // Motion on right only -> move direction left
    dir = LEFT;
    mc = LCONFIG;
  } else if(rPhotoChange>THRESHOLD_FACTOR && lPhotoChange>THRESHOLD_FACTOR) {
    // Motion on both left and right, choose opposite direction of greatest change
    dir = rPhotoChange > lPhotoChange ? LEFT : RIGHT;
    mc = rPhotoChange > lPhotoChange ? LCONFIG : RCONFIG;
  } else {
    // No motion detected so return
    movedPrevIter = false;
    return;
  }

  // Log motor direction
  if(dir == LEFT) {
    Serial.print("Motion detected on right, moving left!\n");
  } else {
    Serial.print("Motion detected on left, moving right!\n");
  }
  
  // Spin initial direction.
  spin(mc,dir);
  turnOff(mc);

  // Delay.
  delayMicroseconds(PAUSE_TIME_MICRO);

  // Spin opposite initial direction.
  spin(mc,!dir);
  turnOff(mc);

  movedPrevIter = true;
}
