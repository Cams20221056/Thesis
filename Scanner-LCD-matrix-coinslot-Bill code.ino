#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <WiFi.h>
#include <HttpClient.h>
#include <DIYables_Keypad.h>  // DIYables_Keypad library

// Wi-Fi Credentials
const char *ssid = "Adik";
const char *password = "Adik12345678";

// Server details
const char *server = "192.168.152.20";
const int port = 80;

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 20, 4); // 20x4 LCD

// Define pins
#define COIN_COUNTER_PIN 2
#define BILL_ACCEPTOR_PIN 3
#define SERVO_PIN 12

// Keypad configuration using DIYables_Keypad library
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

// Variables for balance and coin/bill detection
volatile int coinPulseCount = 0;
volatile int billPulseCount = 0;
int balance = 0;
int selectedItemPrice = 0;
int selectedItemIndex = -1;

// Coin values
const int onePeso = 1, fivePeso = 5, tenPeso = 10, twentyPeso = 20;
const int itemPrices[] = {5, 10, 15, 20, 25, 30, 35, 40, 45, 50};
const bool itemAvailability[] = {true, true, true, true, true, true, true, true, true, true};

// Servo setup
Servo myServo;

// WiFi and HTTP Client
WiFiClient client;
HttpClient httpClient(client, server, port);

// Phase flags
bool phase0 = true, phase1 = false, phase2 = false, phase3 = false, phase4 = false;

void setup() {
  lcd.init();
  lcd.backlight();
  
  Serial.begin(115200);
  Serial1.begin(9600); // Barcode scanner

  // Wi-Fi Connection
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(2000);
  lcd.clear();

  // Set up interrupts
  pinMode(COIN_COUNTER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(COIN_COUNTER_PIN), countCoinPulse, FALLING);

  pinMode(BILL_ACCEPTOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BILL_ACCEPTOR_PIN), countBillPulse, FALLING);

  myServo.attach(SERVO_PIN);
  
  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Please Scan Barcode");
}

void loop() {
  if (Serial1.available()) {
    String scannedData = Serial1.readStringUntil('\n');
    scannedData.trim();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");
    delay(1000);

    // Send the request
    String url = String("/SD333/serverside/check_user.php?user_id=") + scannedData;
    httpClient.get(url);
    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    // Handle the response
    if (statusCode == 200 && response.startsWith("exists,")) {
      displayWelcomeMessage();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No Record Found!");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please Scan Barcode");
    }
  }

  char key = keypad.getKey();
  if (key == 'A') {
    lcd.clear();
    processCashMode();
  } else if (key == 'B') {
    lcd.clear();
    processPointsMode();
  } else if (key == 'C' && phase1) {  // Cancel button functionality in cash mode
    phase1 = false;
    phase0 = true;
    displayWelcomeMessage();
  }

  if (coinPulseCount > 0) {
    processCoinValue(coinPulseCount);
    coinPulseCount = 0;
  }
  if (billPulseCount > 0) {
    processBillValue(billPulseCount);
    billPulseCount = 0;
  }

  if (phase2) cashInputPhase();
  else if (phase3) itemSelectionPhase();
  else if (phase4) dispenseItemPhase();
}

void processCashMode() {
  phase0 = false;
  phase1 = true;
  lcd.setCursor(0, 0);
  lcd.print("Insert Coins/Bills");
  lcd.setCursor(0, 1);
  lcd.print("Press C for cancel");
}

void processPointsMode() {
  phase0 = false;
  phase1 = true;
  lcd.setCursor(0, 0);
  lcd.print("Fetching Points...");
  
  // Assuming the scannedData is still available from the barcode scan
  String scannedData = ""; // You need to store the scannedData globally or pass it appropriately
  String url = String("/SD333/serverside/get_points.php?user_id=") + scannedData; // Adjust the URL as needed
  httpClient.get(url);
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  if (statusCode == 200) {
    // Assuming the response is in the format "points,<points_value>"
    if (response.startsWith("points,")) {
      String pointsStr = response.substring(7); // Extract points value
      int points = pointsStr.toInt();
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Your Points: ");
      lcd.setCursor(0, 1);
      lcd.print(points);
      lcd.setCursor(0, 2);
      lcd.print("Press C to cancel");
      
      // Wait for user to press 'C' to cancel
      while (true) {
        char key = keypad.getKey();
        if (key == 'C') {
          displayWelcomeMessage();
          break; // Exit the loop and return to welcome message
        }
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error fetching");
      lcd.setCursor(0, 1);
      lcd.print("points!");
      delay(2000);
      displayWelcomeMessage();
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error connecting");
    delay(2000);
    displayWelcomeMessage();
  }
}

void displayWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to our");
  lcd.setCursor(0, 1);
  lcd.print("Vending Machine");
  lcd.setCursor(0, 2);
  lcd.print("Press A for cash");
  lcd.setCursor(0, 3);
  lcd.print("Press B for points");
}

void cashInputPhase() {
  updateLCD();
  if (balance > 0) {
    phase2 = false;
    phase3 = true;
  }
}

void itemSelectionPhase() {
  lcd.setCursor(0, 1);
  lcd.print("Select item (1-10)");
  char key = keypad.getKey();
  if (key >= '1' && key <= '9') {
    int selectedItem = key - '0';
    if (balance >= itemPrices[selectedItem - 1] && itemAvailability[selectedItem - 1]) {
      selectedItemIndex = selectedItem - 1;
      selectedItemPrice = itemPrices[selectedItemIndex];
      lcd.setCursor(0, 2);
      lcd.print("Dispensing...");
      phase3 = false;
      phase4 = true;
    } else {
      lcd.setCursor(0, 3);
      lcd.print("Insufficient funds");
      delay(1000);
    }
  }
}

void dispenseItemPhase() {
  myServo.write(180);
  delay(2000);
  myServo.write(90);

  int change = balance - selectedItemPrice;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispensed Item!");
  if (change > 0) {
    lcd.setCursor(0, 1);
    lcd.print("Change: ");
    lcd.print(change);
    lcd.print(" PHP");
  }
  delay(2000);
  resetTransactionData();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Please Scan Barcode");
}

void resetTransactionData() {
  balance = 0;
  selectedItemPrice = 0;
  selectedItemIndex = -1;
  phase4 = false;
}

void processCoinValue(int pulses) {
  int coinValue = (pulses == 1) ? onePeso : (pulses == 5) ? fivePeso : (pulses == 10) ? tenPeso : (pulses == 20) ? twentyPeso : 0;
  balance += coinValue;
  updateLCD();
}

void processBillValue(int pulses) {
  balance += pulses * 10;
  updateLCD();
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Balance: ");
  lcd.print(balance);
  lcd.print(" PHP");
}

void countCoinPulse() {
  static unsigned long lastCoinTime = 0;
  if (micros() - lastCoinTime > 50000) {
    coinPulseCount++;
    lastCoinTime = micros();
  }
}

void countBillPulse() {
  static unsigned long lastBillTime = 0;
  if (millis() - lastBillTime > 500) {
    billPulseCount++;
    lastBillTime = millis();
  }
