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
const MotorConfig rConfig={.stepPin=D1, .dirPin=D2}; // RIGHT
const MotorConfig lConfig={.stepPin=D3, .dirPin=D4}; // LEFT
const unsigned int photo = A0;

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

// Number of test iterations used to compute photo baseline.
const unsigned int PHOTO_BASELINE_ITER_MAX = 20;

// Fraction of photo baseline to use as threshold.
const float THRESHOLD_FACTOR = 0.05;

////////////////////////////////////////
// Photo state
////////////////////////////////////////

unsigned int photoBaseline = 0;
unsigned int photoBaselineIter = 0;
bool photoBaselineSet = false;

////////////////////////////////////////
// Photo functions
////////////////////////////////////////

void updatePhotoBaseline(unsigned int photoIntensity) {
  if(photoBaselineSet) {
    return;
  }
  photoBaseline += photoIntensity;
  ++photoBaselineIter;
  if(photoBaselineIter==PHOTO_BASELINE_ITER_MAX) {
    photoBaseline = photoBaseline/photoBaselineIter;
    Serial.print("photo baseline set to: ");
    Serial.print(photoBaseline);
    Serial.println();
    photoBaselineSet = true;
    photoBaselineIter = 0;
  }   

  // Spread out the samples over time.
  delay(100);
}

bool motionOccurred(unsigned int photoIntensity) {
  return abs(photoIntensity-photoBaseline)>THRESHOLD_FACTOR*photoBaseline;
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
  pinMode(rConfig.stepPin,OUTPUT);
  pinMode(rConfig.dirPin,OUTPUT);
  pinMode(lConfig.stepPin,OUTPUT);
  pinMode(lConfig.dirPin,OUTPUT);
}

////////////////////////////////////////
// Main loop
////////////////////////////////////////

void loop() {
  // Read current photo value
  const unsigned int photoIntensity = analogRead(photo);
  
  // Update baseline
  updatePhotoBaseline(photoIntensity);
  if(!photoBaselineSet) {
    return;
  }

  // Determine if motion occurred
  const bool motion = motionOccurred(photoIntensity);
  if(!motion) {
    return;
  } else {
    Serial.print("Motion detected!\n");
  }

  // Determine initial direction.
  // FixMe: hardcoded to RIGHT for now
  const unsigned int dir = RIGHT;
  const MotorConfig mc = rConfig;

  // Spin initial direction.
  spin(mc,dir);
  turnOff(mc);

  // Delay.
  delayMicroseconds(PAUSE_TIME_MICRO);

  // Spin opposite initial direction.
  spin(mc,!dir);
  turnOff(mc);
}
