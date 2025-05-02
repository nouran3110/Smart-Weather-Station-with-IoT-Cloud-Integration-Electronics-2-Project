#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

const char* ssid = "Network Name";
const char* password = "Network_Password";

const char* server = "http://api.thingspeak.com/update";
String apiKey = "Your API Key on ThingSpeak"; 

float temperature;
float humidity;
float pressure;
float altitude;

int wifiFailCount = 0; 

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    wifiFailCount = 0; 
  } else {
    Serial.println("\nFailed to connect to WiFi!");
    wifiFailCount++;
    if (wifiFailCount >= 3) {
      Serial.println("Restarting ESP32 due to repeated WiFi failure...");
      delay(2000);
      ESP.restart();
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  bmp.begin();
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    connectWiFi();
  }

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  pressure = bmp.readPressure() / 100.0;
  altitude = bmp.readAltitude();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.println("Sending data to ThingSpeak...");

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = server;
      url += "?api_key=" + apiKey;
      url += "&field1=" + String(temperature);
      url += "&field2=" + String(humidity);
      url += "&field3=" + String(pressure);
      url += "&field4=" + String(altitude);

      http.begin(url);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("Data sent. Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error sending data. Code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
  }

  delay(15000); 
}