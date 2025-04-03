#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define CLOCKWISE 250   // Adjust this value as needed (try 250, 275, 300)
#define COUNTERCLOCKWISE 450  // Just for reference

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(50); // Standard servo frequency
}

void loop() {
  for (int channel = 4; channel <= 11; channel++) {
    pwm.setPWM(channel, 0, COUNTERCLOCKWISE); // Rotate clockwise
  }
  delay(2000); // Keep rotating
}