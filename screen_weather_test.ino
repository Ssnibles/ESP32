#include <TFT_eSPI.h>
#include <SPI.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

#define TFT_BL   21  // GPIO pin for backlight control

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";
// Replace with your OpenWeatherMap API key
const char* apiKey = "API_KEY";
// Replace with your city and country code
const char* city = "POST_CODE";
const char* countryCode = "NZ";

unsigned long lastTime = 0;
unsigned long delayVal = 5000;


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi with ssid: ");
  Serial.println(ssid);
  Serial.print("and IP Address: ");
  Serial.println(WiFi.localIP());

  tft.init();
  tft.setRotation(0);
  
  // Set up backlight control
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // Turn on backlight
  // Alternatively, for PWM control:
  // ledcSetup(0, 5000, 8);  // Channel 0, 5000 Hz, 8-bit resolution
  // ledcAttachPin(TFT_BL, 0);  // Attach TFT_BL pin to channel 0
  // ledcWrite(0, 255);  // Set brightness to maximum (0-255)

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("Hi Mum!", 10, 10);
  tft.drawString("You're the greatest >:3", 10, 30);
}

void loop() {
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

      // Access and convert JSON elements
      JsonObject weather = doc["weather"][0];
      float temperatureK = doc["main"]["temp"];
      float temperatureC = temperatureK - 273.15;  // Convert from Kelvin to Celsius
      float pressure = doc["main"]["pressure"];
      int humidity = doc["main"]["humidity"];
      float windSpeed = doc["wind"]["speed"];
      int windDirection = doc["wind"]["deg"];
      const char* weatherConditions = weather["main"];
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
      Serial.println(weatherConditions);
      Serial.print("Weather Description: ");
      Serial.println(weatherDescription);
      Serial.println("____________________________________________________________");
    }
  }
  lastTime = millis();
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
