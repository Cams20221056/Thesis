#include <Servo.h>

Servo servo1;  // First servo
Servo servo2;  // Second servo

const int servo1Pin = 2;
const int servo2Pin = 3;
const int rotationTime = 500; // Time in milliseconds

void setup() {
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);
}

void loop() {
    // Rotate servos in opposite directions
    servo1.write(90);  // Adjust values based on your servo type
    servo2.write(90);  // Adjust values based on your servo type (reverse direction)
    delay(rotationTime);

    // Stop servos
    servo1.write(0);
    servo2.write(180); // Reverse stop position
    delay(100000); // Pause before next cycle
}