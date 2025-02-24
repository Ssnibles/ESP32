#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";
// Replace with your OpenWeatherMap API key
const char* apiKey = "API_KEY";
// Replace with your city and country code
const char* city = "3112";
const char* countryCode = "NZ";

unsigned long lastTime = 0;
unsigned long delayVal = 10000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
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
      float temperatureK = doc["main"]["temp"];
      float temperatureC = temperatureK - 273.15;  // Convert from Kelvin to Celsius
      float pressure = doc["main"]["pressure"];
      int humidity = doc["main"]["humidity"];
      float windSpeed = doc["wind"]["speed"];
      int windDirection = doc["wind"]["deg"];
      String weatherConditions = String(doc["weather"]["main")];

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

      // if (temperatureC > 10) {
      //   Serial.println("It is above ten degrees C");
      // }
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
