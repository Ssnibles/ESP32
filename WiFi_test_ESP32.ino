/*
  This is ESP32 WiFi test code, it will try to connect your specified SSID and check for any errors, it will then print the WiFi status into the terminal

  WiFi status codes:
  WL_IDLE_STATUS (0): WiFi is in an idle state.
  WL_NO_SSID_AVAIL (1): No SSID is available.
  WL_SCAN_COMPLETED (2): Scan networks is completed.
  WL_CONNECTED (3): Successfully connected to a WiFi network.
  WL_CONNECT_FAILED (4): Failed to connect to a WiFi network.
  WL_CONNECTION_LOST (5): Connection was lost.
  WL_DISCONNECTED (6): Disconnected from the WiFi network.
*/

#include <WiFi.h>

#define BUILTIN_LED_PIN 2  // Pin number for the built-in LED


// Replace with your network credentials
const char* ssid = "SSID";          // Your SSID
const char* password = "PASSWORD";  // Your password

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password); // Initilise WiFi with given SSID and password
  Serial.println("Connecting to WiFi");

  pinMode(BUILTIN_LED_PIN, OUTPUT);  // Set the built-in LED pin as output

  // While the WiFi is disconnected
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    int status = WiFi.status();
    Serial.print("WiFi Status: ");
    Serial.println(status);

    switch (status) {
      case WL_NO_SSID_AVAIL:
        Serial.println("No SSID available. Check your SSID.");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("Connection failed. Check your credentials.");
        break;
      case WL_CONNECTION_LOST:
        Serial.println("Connection lost. Trying to reconnect...");
        break;
      case WL_DISCONNECTED:
        Serial.println("Disconnected from WiFi. Trying to reconnect...");
        break;
      default:
        Serial.println("Unknown status code.");
        break;
    }
    digitalWrite(BUILTIN_LED_PIN, HIGH);  // Light Built-In LED while not connected to WiFi
  }

  digitalWrite(BUILTIN_LED_PIN, LOW);  // Turn off the built-in LED when connected to WiFi
  Serial.println("Successfully connected to WiFi");
  Serial.print("WiFi IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Your main code here
}
