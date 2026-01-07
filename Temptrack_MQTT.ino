#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Sensor Configuration ---
#define ONE_WIRE_BUS 19  // Data wire connected to GPIO 19
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- WiFi Configuration ---
const char* ssid = "Nenae";        
const char* password = "nenae014"; 

// --- MQTT Broker Configuration ---
const char* mqtt_server = "13.212.111.239"; // AWS EC2 IP

// --- Time Server Settings (NTP) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;   // GMT+7 for Thailand
const int   daylightOffset_sec = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// --- Timer Variables for Non-blocking Delay ---
unsigned long previousMillis = 0;  // Stores the last time temperature was published
const long interval = 5000;        // Interval at which to publish (5000ms = 5s)

void setup() {
  Serial.begin(115200);

  // Initialize Sensor
  sensors.begin();
  Serial.println("DS18B20 Sensor Started");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Sync Time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
     Serial.println("Failed to obtain time initially");
  } else {
     Serial.println("Time Synced!");
  }

  // Setup MQTT
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "null"; 
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

void loop() {
  // 1. Maintain MQTT Connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 2. Check if 5 seconds have passed (Non-blocking timer)
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // Save the last time we sent data
    previousMillis = currentMillis;

    // --- Start Reading and Sending Process ---
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures(); 
    float realTemp = sensors.getTempCByIndex(0); 

    // Validate Sensor Reading
    if(realTemp == -127.00) {
      Serial.println("Error: Could not read temperature data");
    } else {
      Serial.print("Temperature is: ");
      Serial.println(realTemp);

      // Prepare Payload
      String timeStr = getCurrentTime();
      String payload = "{\"sensor_id\": \"ESP32_01\", \"temp\": " + String(realTemp) + ", \"timestamp\": \"" + timeStr + "\"}";
      
      Serial.print("Sending payload: ");
      Serial.println(payload);
      
      // Publish to MQTT
      client.publish("kidbright/temp", payload.c_str());
    }
  }
  // No delay() here! The loop runs continuously to keep MQTT alive.
}
