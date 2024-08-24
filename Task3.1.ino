#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <BH1750.h>

// WiFi credentials
char ssid[] = "OnePlus";     // your network SSID (name)
char pass[] = "pt123456789"; // your network password

// IFTTT credentials
const char* iftttEventName1 = "sunlight_detected"; // Event name for light detected
const char* iftttEventName2 = "sunlight_stopped";  // Event name for light stopped
const char* iftttKey = "jHW-ss7Vj2vsfSTttSZVqb6fbS2ucmKUW5quCfDJoRw";  // IFTTT Webhook Key

const int lightThresholdDetect = 800; // Threshold to send "detected" message
const int lightThresholdStop = 200;  // Threshold to send "stopped" message
BH1750 lightMeter;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, "maker.ifttt.com", 80);

bool lightAboveThreshold = false; // Track the previous state

void setup() {
  Serial.begin(9600);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    delay(5000);
  }
  Serial.println("Connected to WiFi");

  Wire.begin();
  if (!lightMeter.begin()) {
    Serial.println("Error initializing BH1750. Check your connections.");
    while (1);
  }
}

void loop() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Light Level: ");
  Serial.print(lux);
  Serial.println(" lux");

  if (lux > lightThresholdDetect && !lightAboveThreshold) {
    // if light level crossed above the detection threshold
    sendIFTTTRequest(lux, "detected");
    lightAboveThreshold = true;
  } else if (lux <= lightThresholdStop && lightAboveThreshold) {
    // if light level has dropped below the stop threshold
    sendIFTTTRequest(lux, "stopped");
    lightAboveThreshold = false;
  }

  delay(1000); // Check light intensity every second
}

void sendIFTTTRequest(float lux, const char* status) {
  // Choose the  event name based on the status
  String eventName = (strcmp(status, "detected") == 0) ? iftttEventName1 : iftttEventName2;
  
  String url = "/trigger/" + eventName + "/with/key/" + String(iftttKey);
  url += "?value1=" + String(lux) + "&value2=" + String(status);
  
  Serial.print("Sending request to IFTTT: ");
  Serial.println(url);
  
  client.beginRequest();
  client.get(url);
  client.endRequest();
  
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}
