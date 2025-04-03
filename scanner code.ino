#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HttpClient.h>

// Replace with your network credentials
const char *ssid = "Adik";
const char *password = "Adik12345678";

// Server details (IP address or domain)
const char *server = "192.168.152.20";  // Replace with your server's IP or domain
const int port = 80;  // Default HTTP port

// Initialize the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Initialize WiFi and HTTP client
WiFiClient client;
HttpClient httpClient(client, server, port);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to initialize

  Serial1.begin(9600); // For barcode scanner communication

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Scanner Ready");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(2000);
  lcd.clear();
}

void loop() {
  if (Serial1.available()) {
    String scannedData = Serial1.readStringUntil('\n'); // Read barcode data
    scannedData.trim(); // Remove leading/trailing whitespace
    Serial.println("Scanned Data: " + scannedData); // Print to Serial Monitor

    // Display scanned data on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanning...");
    delay(1000);

    // Prepare the URL for the HTTP request to check the database
    String url = String("/SD333/serverside/check_user.php?user_id=") + scannedData;

    // Send the HTTP GET request
    httpClient.get(url);

    // Get the HTTP response
    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();

    // Handle response
    if (statusCode == 200) {
      if (response.startsWith("exists,")) {
        String points = response.substring(response.indexOf(",") + 1); // Extract points
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("User Found");
        lcd.setCursor(0, 1);
        lcd.print("Points: " + points);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("User Not Found");
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error: ");
      lcd.setCursor(0, 1);
      lcd.print("Code: " + String(statusCode));
    }

    delay(3000); // Wait before allowing next scan
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scanner Ready");
  }
}