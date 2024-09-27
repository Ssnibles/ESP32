/*
  WiFi status codes:
  WL_IDLE_STATUS (0): WiFi is in an idle state.
  WL_NO_SSID_AVAIL (1): No SSID is available.
  WL_SCAN_COMPLETED (2): Scan networks is completed.
  WL_CONNECTED (3): Successfully connected to a WiFi network.
  WL_CONNECT_FAILED (4): Failed to connect to a WiFi network.
  WL_CONNECTION_LOST (5): Connection was lost.
  WL_DISCONNECTED (6): Disconnected from the WiFi network.
*/

#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

TFT_eSPI tft = TFT_eSPI();

#define BL_PIN 19       // GPIO pin for backlight control
#define BUILT_IN_LED 2  // Pin number for the built-in LED
#define CIRCLE_PIN 21   // LED circle pin
#define NUMPIXELS 24    // Number of LEDs

Adafruit_NeoPixel pixels(NUMPIXELS, CIRCLE_PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char* ssid = "Blub";
const char* password = "Fabi2006";
// Replace with your OpenWeatherMap API key
const char* apiKey = "bda8f719480d16fda6dda1d986e25675";
// Replace with your city and country code
const char* city = "3112";       // Postal code for location
const char* countryCode = "NZ";  // Weather grab country code

unsigned long lastTime = 0;
unsigned long delayVal = 120000;  // Delay between weather grabs, don't put this too hight to not drain credit

void setup() {
  Serial.begin(115200);

  pixels.begin();  // Begin

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");

  pinMode(BUILT_IN_LED, OUTPUT);  // Set the built-in LED pin as output
  pinMode(27, OUTPUT);

  // While the WiFi is disconnected
  while (WiFi.status() != WL_CONNECTED) {
//    delay(100);
    int status = WiFi.status();
    Serial.print("WiFi Status: ");
    Serial.println(status);

    digitalWrite(BUILT_IN_LED, HIGH);  // Light Built-In LED while not connected to WiFi

//    digitalWrite(27, HIGH);

    // Red spinning circle while not connected
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0));
      pixels.show();
      delay(30);
      pixels.clear();
    }
  }

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    pixels.show();
    delay(10);
  }

  digitalWrite(BUILT_IN_LED, LOW);  // Turn off the built-in LED when connected to WiFi
  Serial.println("Successfully connected to WiFi");
  Serial.print("WiFi IP address: ");
  Serial.println(WiFi.localIP());

  tft.init();
  tft.setRotation(0);

  // Set up backlight control
  pinMode(BL_PIN, OUTPUT);
  digitalWrite(BL_PIN, HIGH);  // Turn on backlight
  // Alternatively, for PWM control:
  // ledcSetup(0, 5000, 8);  // Channel 0, 5000 Hz, 8-bit resolution
  // ledcAttachPin(TFT_BL, 0);  // Attach TFT_BL pin to channel 0
  // ledcWrite(0, 255);  // Set brightness to maximum (0-255)

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
}

void loop() {
  pixels.clear();

  if ((millis() - lastTime) > delayVal) {
    if (WiFi.status() == WL_CONNECTED) {
      // Make the HTTP GET request
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(countryCode) + "&appid=" + String(apiKey);
      String jsonResponse = httpGETRequest(serverPath.c_str());

      // Deserialize the JSON response
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, jsonResponse);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      digitalWrite(27, HIGH);

      // Access and convert JSON elements
      JsonObject weather = doc["weather"][0];
      float temperatureK = doc["main"]["temp"];
      float temperatureC = temperatureK - 273.15;  // Convert from Kelvin to Celsius
      float pressure = doc["main"]["pressure"];
      int humidity = doc["main"]["humidity"];
      float windSpeed = doc["wind"]["speed"];
      int windDirection = doc["wind"]["deg"];
      const char* weatherCondition = weather["main"];
      const char* weatherDescription = weather["description"];

      // Print the values
      Serial.print("Temperature (C): ");
      Serial.println(temperatureC);
      Serial.print("Pressure (hPa): ");
      Serial.println(pressure);
      Serial.print("Humidity (%): ");
      Serial.println(humidity);
      Serial.print("Wind Speed (m/s): ");
      Serial.println(windSpeed);
      Serial.print("Wind Direction (degrees): ");
      Serial.println(windDirection);
      Serial.print("Weather Conditions: ");
      Serial.println(weatherCondition);
      Serial.print("Weather Description: ");
      Serial.println(weatherDescription);
      Serial.println("____________________________________________________________");

      // Print the specified weather info to the screen, the sizes are specifically for a 2.4" display
      tft.drawString("Temperature: " + String(temperatureC), 10, 10);
      tft.drawString("Pressure: " + String(pressure), 10, 35);
      tft.drawString("Humidity: " + String(humidity), 10, 55);
      tft.drawString("Wind Speed: " + String(windSpeed), 10, 75);
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
  http.begin(serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0) {
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}