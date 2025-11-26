#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Firebase addons
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ----- WiFi Credentials -----
const char* WIFI_SSID = "ZhChiah's Iphone";
const char* WIFI_PASS = "chiah256256";

// ----- Firebase Project Credentials -----
#define API_KEY       "AIzaSyBuq5yWweXCz1fCxEx5pwRqJgzRK9aX9w4" // <-- put your Web API key here
#define DATABASE_URL  "https://monitoring-sauce-level-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ----- Pins & LCD -----
const int trigPin = 2;
const int echoPin = 15;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ----- Functions -----
long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2; // cm
}

void setup() {
  Serial.begin(115200);

  // --- LCD Startup Test ---
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  delay(2000);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // --- WiFi ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(1500);

  // --- Firebase ---
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase SignUp OK");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Firebase OK");
  } else {
    Serial.printf("SignUp Failed: %s\n", config.signer.signupError.message.c_str());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Firebase FAIL");
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  delay(2000);
  lcd.clear();
}

void loop() {
  // ---- Take and Display readings one by one (REAL-TIME) ----
  
  // Reading 1
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taking Reading 1...");
  long d1 = getDistance();
  delay(300);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading 1:");
  lcd.setCursor(0, 1);
  lcd.print(d1); lcd.print(" cm");
  delay(2000);  // Time to read the result
  
  lcd.clear();
  lcd.print("Next reading...");
  delay(1000);  // Pause before next reading

  // Reading 2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taking Reading 2...");
  long d2 = getDistance();
  delay(300);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading 2:");
  lcd.setCursor(0, 1);
  lcd.print(d2); lcd.print(" cm");
  delay(2000);  // Time to read the result
  
  lcd.clear();
  lcd.print("Next reading...");
  delay(1000);  // Pause before next reading

  // Reading 3
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Taking Reading 3...");
  long d3 = getDistance();
  delay(300);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading 3:");
  lcd.setCursor(0, 1);
  lcd.print(d3); lcd.print(" cm");
  delay(2000);  // Time to read the result
  
  lcd.clear();
  lcd.print("Calculating...");
  delay(1000);  // Pause before showing result

  int distance = (d1 + d2 + d3) / 3; // Average
  String status;

  if (distance >= 13) status = "Need Refill";
  else if (distance >= 10) status = "Level Low";
  else status = "Level OK";

  // ---- Serial Print ----
  Serial.printf("R1:%ld  R2:%ld  R3:%ld  Avg:%d  -> %s\n", d1, d2, d3, distance, status.c_str());

  // ---- Show Final Result ----

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg: "); lcd.print(distance); lcd.print(" cm");
  lcd.setCursor(0, 1);
  lcd.print("Status: "); lcd.print(status);
  delay(3000);  // Extra time for final important result

  // ---- Send to Firebase ----
  if (Firebase.ready()) {
    Firebase.RTDB.setInt(&fbdo, "/SensorData/R1_cm", d1);
    Firebase.RTDB.setInt(&fbdo, "/SensorData/R2_cm", d2);
    Firebase.RTDB.setInt(&fbdo, "/SensorData/R3_cm", d3);
    Firebase.RTDB.setInt(&fbdo, "/SensorData/Avg_cm", distance);
    Firebase.RTDB.setString(&fbdo, "/SensorData/Alert", status);
    Firebase.RTDB.setInt(&fbdo, "/SensorData/Timestamp", millis()/1000);
  }
}