/*
This code will grab a JSON array from OpenWeatherMaps using the Arduino WiFi.h library, decode it using the ArduinoJson.h library, and will display select data on a 2.4” OLED display using the TFT_eSPI and SPI.h libraries.
The current weather condition, such as; clouds, clear, rain etc, will then be reflected through a 24 LED NeoPixel ring.
Effects include; A blue rain drop effect, a yellow-orange radiant sunbeam, a grey slightly shifting effect to simulate cloud, etc.
These effects are declared as void functions to be easily declared, extended, and to allow for better organisation and reuse in the code anywhere.
The current brightness of the LEDs can also be controlled through a web server using the WiFi.h library to display a static website. The website contains; The Name of the project (ESP32 Weather Station), the current brightness in percentage, one button to turn the brightness OFF (zero), one button to turn the brightness ON (equivalent to full brightness), and a slider to adjust the brightness to any percentage up to 100. It also contains a button to set this current brightness.
The website will only be accessible on the same network as the ESP, unless port forwarding is set up in the routers setting, but some ISPs do not allow for this.
All ISPs do allow setting a device's IP address to a constant one, which is recommended to have the website easily accessible from the same network without having to look up the new IP every time. Below is an image of how this might look like.

  WiFi status codes:
  WL_IDLE_STATUS (0): WiFi is in an idle state.
  WL_NO_SSID_AVAIL (1): No SSID is available.
  WL_SCAN_COMPLETED (2): Scan networks is completed.
  WL_CONNECTED (3): Successfully connected to a WiFi network.
  WL_CONNECT_FAILED (4): Failed to connect to a WiFi network.
  WL_CONNECTION_LOST (5): Connection was lost.
  WL_DISCONNECTED (6): Disconnected from the WiFi network.
*/

// Include necessary libraries
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// Constants
#define BL_PIN 19
#define BUILT_IN_LED 2
#define CIRCLE_PIN 21
#define NUMPIXELS 24

// WiFi and API settings
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* apiKey = "API_KEY";
const char* city = "POSTAL_CODE";
const char* countryCode = "NZ";

// Global variables
TFT_eSPI tft = TFT_eSPI();
Adafruit_NeoPixel pixels(NUMPIXELS, CIRCLE_PIN, NEO_GRB + NEO_KHZ800);
WiFiServer server(80);
String header;
unsigned long lastEffectUpdate = 0;
const long effectInterval = 50;
int effectStep = 0;
uint8_t brightness = 127;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 180000;  // 3 minutes
String weatherMain = "Clear";
String weatherDescription = "clear sky";

// Function prototypes
void setupDisplay();
void connectToWiFi();
void fetchWeatherData();
void handleWebClient(WiFiClient client);
void updateWeatherEffect();
void displayWeatherData(float temp, float pressure, int humidity, float windSpeed, const char* conditions);
String httpGETRequest(const char* serverName);
void notConnected();
void setBrightness(uint8_t newBrightness);
void sunEffect();
void cloudEffect();
void rainEffect();
void thunderstormEffect();
void defaultEffect();

void setup() {
  Serial.begin(115200);
  setupDisplay();
  connectToWiFi();
  fetchWeatherData();
  lastWeatherUpdate = millis();
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Update weather effect animation
  if (currentMillis - lastEffectUpdate >= effectInterval) {
	lastEffectUpdate = currentMillis;
	updateWeatherEffect();
  }

  // Fetch new weather data every 3 minutes
  if (currentMillis - lastWeatherUpdate >= weatherUpdateInterval) {
	fetchWeatherData();
	lastWeatherUpdate = currentMillis;
  }

  // Handle web client requests
  WiFiClient client = server.available();
  if (client) {
	handleWebClient(client);
  }
}

// Initialize and set up the display
void setupDisplay() {
  pixels.begin();
  pixels.setBrightness(brightness);
  pinMode(BUILT_IN_LED, OUTPUT);
  pinMode(27, OUTPUT);
  tft.init();
  tft.setRotation(0);
  pinMode(BL_PIN, OUTPUT);
  digitalWrite(BL_PIN, HIGH);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
}

// Connect to WiFi network
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
	Serial.print(".");
	notConnected();
	if (dotCount == 0) {
  	tft.fillRect(15, 110, tft.width() - 30, 20, TFT_BLACK);
  	tft.setCursor(15, 110);
  	tft.print("Connecting to WiFi");
	}
	tft.print(".");
	dotCount = (dotCount + 1) % 3;
	delay(500);
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  pixels.clear();
  pixels.fill(pixels.Color(0, 255, 0), 0, NUMPIXELS);
  pixels.show();
  delay(500);
  pixels.clear();
  pixels.show();
}

// Fetch weather data from OpenWeatherMap API
void fetchWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
	String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "," + String(countryCode) + "&appid=" + String(apiKey);
	String jsonResponse = httpGETRequest(serverPath.c_str());

	StaticJsonDocument<1024> doc;
	DeserializationError error = deserializeJson(doc, jsonResponse);

	if (error) {
  	Serial.print(F("deserializeJson() failed: "));
  	Serial.println(error.f_str());
  	return;
	}

	// Extract weather data from JSON response
	JsonObject weather = doc["weather"][0];
	float temperatureK = doc["main"]["temp"];
	float temperatureC = temperatureK - 273.15;
	float pressure = doc["main"]["pressure"];
	int humidity = doc["main"]["humidity"];
	float windSpeedms = doc["wind"]["speed"];
	float windSpeed = windSpeedms * 3.6;
	weatherMain = weather["main"].as<String>();
	weatherDescription = weather["description"].as<String>();

	// Print extracted data
	Serial.println("\nExtracted Weather Data:");
	Serial.printf("Temperature: %.2f°C (%.2f K)\n", temperatureC, temperatureK);
	Serial.printf("Pressure: %.2f hPa\n", pressure);
	Serial.printf("Humidity: %d%%\n", humidity);
	Serial.printf("Wind Speed: %.2f m/s (%.2f km/h)\n", windSpeedms, windSpeed);
	Serial.printf("Weather Main: %s\n", weatherMain.c_str());
	Serial.printf("Weather Description: %s\n", weatherDescription.c_str());

	displayWeatherData(temperatureC, pressure, humidity, windSpeed, weatherDescription.c_str());
  }
}

// Handle web client requests
void handleWebClient(WiFiClient client) {
  String currentLine = "";
  while (client.connected()) {
	if (client.available()) {
  	char c = client.read();
  	header += c;
  	if (c == '\n') {
    	if (currentLine.length() == 0) {
      	// HTTP headers always start with a response code and a content-type
      	client.println("HTTP/1.1 200 OK");
      	client.println("Content-type:text/html");
      	client.println("Connection: close");
      	client.println();

      	// Handle brightness control requests
      	if (header.indexOf("GET /brightness/off") >= 0) {
        	setBrightness(0);
      	} else if (header.indexOf("GET /brightness/on") >= 0) {
        	setBrightness(255);
      	} else if (header.indexOf("GET /brightness/set?value=") >= 0) {
        	int pos = header.indexOf("GET /brightness/set?value=");
        	String brightnessStr = header.substring(pos + 26);
        	brightnessStr = brightnessStr.substring(0, brightnessStr.indexOf(" HTTP"));
        	int brightnessPercent = brightnessStr.toInt();
        	setBrightness(map(brightnessPercent, 0, 100, 0, 255));
      	}

      	// Generate and send HTML page
      	client.println("<!DOCTYPE html><html>");
      	client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
      	client.println("<style>body { font-family: Arial, sans-serif; text-align: center; }");
      	client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
      	client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");
      	client.println(".slider { width: 300px; }</style></head>");
      	client.println("<body><h1>ESP32 Weather Station</h1>");
      	client.println("<p>Current Brightness: <span id='brightnessValue'>" + String(map(brightness, 0, 255, 0, 100)) + "</span>%</p>");
      	client.println("<p><a href=\"/brightness/off\"><button class=\"button\">Turn Off Brightness</button></a>");
      	client.println("<a href=\"/brightness/on\"><button class=\"button\">Turn On Brightness</button></a></p>");
      	client.println("<p><input type=\"range\" min=\"0\" max=\"100\" value=\"" + String(map(brightness, 0, 255, 0, 100)) + "\" class=\"slider\" id=\"brightnessSlider\">");
      	client.println("<button onclick=\"setBrightness()\">Set Brightness</button></p>");

      	// JavaScript for handling brightness slider
      	client.println("<script>");
      	client.println("var slider = document.getElementById('brightnessSlider');");
      	client.println("var output = document.getElementById('brightnessValue');");
      	client.println("output.innerHTML = slider.value;");
      	client.println("slider.oninput = function() { output.innerHTML = this.value; }");
      	client.println("function setBrightness() {");
      	client.println("  var sliderValue = document.getElementById('brightnessSlider').value;");
      	client.println("  var xhr = new XMLHttpRequest();");
      	client.println("  xhr.onreadystatechange = function() {");
      	client.println("	if (this.readyState == 4 && this.status == 200) {");
      	client.println("  	document.getElementById('brightnessValue').innerText = sliderValue;");
      	client.println("  	console.log('Brightness set to: ' + sliderValue + '%');");
      	client.println("	}");
      	client.println("  };");
      	client.println("  xhr.open('GET', '/brightness/set?value=' + sliderValue, true);");
      	client.println("  xhr.send();");
      	client.println("}");
      	client.println("</script>");
      	client.println("</body></html>");
      	break;
    	} else {
      	currentLine = "";
    	}
  	} else if (c != '\r') {
    	currentLine += c;
  	}
	}
  }
  header = "";
  client.stop();
}

// Update weather effect on NeoPixel ring
void updateWeatherEffect() {
  if (weatherMain == "Clear") {
	sunEffect();
  } else if (weatherMain == "null") {
	fetchWeatherData();
  } else if (weatherMain == "Clouds") {
	cloudEffect();
  } else if (weatherMain == "Rain") {
	rainEffect();
  } else if (weatherMain == "Thunderstorm") {
	thunderstormEffect();
  } else {
	defaultEffect();
  }
  pixels.show();
  effectStep = (effectStep + 1) % 360;
}

// Display weather data on TFT screen
void displayWeatherData(float temp, float pressure, int humidity, float windSpeed, const char* conditions) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.printf("Temp: %.1f C", temp);
  tft.setCursor(10, 35);
  tft.printf("Pressure: %.0f hPa", pressure);
  tft.setCursor(10, 60);
  tft.printf("Humidity: %d%%", humidity);
  tft.setCursor(10, 85);
  tft.printf("Wind: %.1f km/h", windSpeed);
  tft.setCursor(10, 110);
  tft.printf("Weather: %s", conditions);
}

// Make HTTP GET request
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

// Display not connected status
void notConnected() {
  pixels.fill(pixels.Color(255, 0, 0), 0, NUMPIXELS);
  pixels.show();
  delay(250);
  pixels.clear();
  pixels.show();
  delay(250);
}

// Set NeoPixel brightness
void setBrightness(uint8_t newBrightness) {
  brightness = newBrightness;
  pixels.setBrightness(brightness);
  pixels.show();
  Serial.println("Brightness set to: " + String(map(brightness, 0, 255, 0, 100)) + "%");
}

// Weather effect functions
void sunEffect() {
  for (int i = 0; i < NUMPIXELS; i++) {
	float angle = (i * 360.0 / NUMPIXELS + effectStep) * 3.14159 / 180.0;
	int brightness = 128 + sin(angle) * 64;
	int r = brightness;
	int g = brightness * 0.7;
	int b = brightness * 0.2;
	pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
}

void cloudEffect() {
  for (int i = 0; i < NUMPIXELS; i++) {
	float angle = (i * 360.0 / NUMPIXELS + effectStep) * 3.14159 / 180.0;
	int brightness = 128 + sin(angle) * 64;
	pixels.setPixelColor(i, pixels.Color(brightness, brightness, brightness));
  }
}

void rainEffect() {
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
	if ((i + effectStep) % 3 == 0) {
  	int blue = random(50, 150);
  	pixels.setPixelColor(i, pixels.Color(0, 0, blue));
	}
  }
}

void thunderstormEffect() {
  for (int i = 0; i < NUMPIXELS; i++) {
	pixels.setPixelColor(i, pixels.Color(20, 20, 30));
  }
  if (random(10) == 0) {
	int lightning = random(50, 255);
	for (int i = 0; i < NUMPIXELS; i++) {
  	pixels.setPixelColor(i, pixels.Color(lightning, lightning, lightning));
	}
  }
}

void defaultEffect() {
  for (int i = 0; i < NUMPIXELS; i++) {
	float angle = (i * 360.0 / NUMPIXELS + effectStep) * 3.14159 / 180.0;
	int value = 128 + sin(angle) * 127;
	pixels.setPixelColor(i, pixels.Color(value, value, value));
  }
}

