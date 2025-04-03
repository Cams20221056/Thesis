change dispensing code:

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <DIYables_Keypad.h>  // DIYables_Keypad library

// PCA9685 Setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define SERVOMIN  100   // Minimum pulse length count (0 degrees)
#define SERVOMAX  500   // Maximum pulse length count (180 degrees)

// Servo Channels
#define SERVO_20 0  // 20 pesos dispenser
#define SERVO_10 1  // 10 pesos dispenser
#define SERVO_5  2  // 5 pesos dispenser
#define SERVO_1  3  // 1 peso dispenser

// Keypad Setup
const int ROW_NUM = 4; 
const int COLUMN_NUM = 4;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {7, 6, 5, 4};  
byte pin_column[COLUMN_NUM] = {11, 10, 9, 8};  

DIYables_Keypad keypad = DIYables_Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Updated Prices
#define ITEM1_PRICE 30  // Updated to 35 pesos
#define ITEM2_PRICE 40  // Updated to 15 pesos
#define ITEM3_PRICE 45  // Updated to 10 pesos
#define ITEM4_PRICE 48  // Unchanged

int balance = 50; // Initial balance

// Function to convert degrees to pulse width for PCA9685
int degreesToPulse(int angle) {
    return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

// Function to dispense change using servos
void dispenseChange(int change) {
    Serial.print("Dispensing change: "); 
    Serial.println(change);

    while (change > 0) {
        if (change >= 20) {
            pwm.setPWM(SERVO_20, 0, degreesToPulse(90)); // Move servo
            delay(200);
            pwm.setPWM(SERVO_20, 0, degreesToPulse(160)); // Reset
            change -= 20;
        } else if (change >= 10) {
            pwm.setPWM(SERVO_10, 0, degreesToPulse(100));
            delay(200);
            pwm.setPWM(SERVO_10, 0, degreesToPulse(160));
            change -= 10;
        } else if (change >= 5) {
            pwm.setPWM(SERVO_5, 0, degreesToPulse(80));
            delay(200);
            pwm.setPWM(SERVO_5, 0, degreesToPulse(160));
            change -= 5;
        } else if (change >= 1) {
            pwm.setPWM(SERVO_1, 0, degreesToPulse(70));
            delay(200);
            pwm.setPWM(SERVO_1, 0, degreesToPulse(160));
            change -= 1;
        }
        delay(500);
    }
}

// Function to handle purchase
void purchaseItem(int price) {
    if (balance >= price) {
        balance -= price;
        Serial.print("Purchased item for ");
        Serial.print(price);
        Serial.println(" pesos.");
        
        int change = balance;
        dispenseChange(change);

        // Reset balance after purchase
        balance = 50;
        Serial.println("Balance reset to 50 pesos.");
        Serial.print("Current Balance: ");
        Serial.println(balance);
    } else {
        Serial.println("Not enough balance.");
    }
}

void setup() {
    Serial.begin(115200);
    pwm.begin();
    pwm.setPWMFreq(50); // 50Hz frequency for servos
    Serial.println("Vending Machine Ready!");
    Serial.print("Initial Balance: ");
    Serial.println(balance);
}

void loop() {
    char key = keypad.getKey();
    
    if (key) {
        Serial.print("Key Pressed: ");
        Serial.println(key);

        if (key == '1') purchaseItem(ITEM1_PRICE);
        else if (key == '2') purchaseItem(ITEM2_PRICE);
        else if (key == '3') purchaseItem(ITEM3_PRICE);
        else if (key == '4') purchaseItem(ITEM4_PRICE);
    }
}