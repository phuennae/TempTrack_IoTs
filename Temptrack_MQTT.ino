#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
// Libraries for the DS18B20 Temperature Sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Sensor Configuration ---
// Data wire is connected to GPIO 19 on the ESP32
#define ONE_WIRE_BUS 19 

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass the oneWire reference to Dallas Temperature library
DallasTemperature sensors(&oneWire);

// --- WiFi Configuration ---
// Update these with the target network credentials
const char* ssid = "..."; // Enter WIFI username        
const char* password = "..."; // Enter WIFI password

// --- MQTT Broker Configuration ---
// IP Address of the AWS EC2 instance acting as the broker
const char* mqtt_server = "..."; // Enter IP of EC2 Example "13.214.25.178"

// --- Time Server Settings (NTP) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;   // Offset for Thailand (GMT+7)
const int   daylightOffset_sec = 0;     // No daylight saving time

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Initialize the DS18B20 sensor library
  sensors.begin();
  Serial.println("DS18B20 Sensor Started");
  
  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize and synchronize time via NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
     Serial.println("Failed to obtain time initially");
  } else {
     Serial.println("Time Synced!");
  }

  // Set the MQTT server and port (default 1883)
  client.setServer(mqtt_server, 1883);
}

// Function to handle MQTT reconnection logic
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random Client ID to avoid conflicts with other devices
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect using the generated Client ID
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      // If connection fails, print the error code and wait 5 seconds before retrying
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Helper function to get the current time as a formatted String
String getCurrentTime() {
  struct tm timeinfo;
  // Return "null" if time cannot be fetched
  if(!getLocalTime(&timeinfo)){
    return "null"; 
  }
  // Format time as: YYYY-MM-DD HH:MM:SS
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

void loop() {
  // Ensure the client is connected to the MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Keep the MQTT client active

  // --- Read Data from DS18B20 Sensor ---
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send command to sensor to convert temperature
  float realTemp = sensors.getTempCByIndex(0); // Read temperature from the first sensor (Index 0)

  // Validate the reading (-127 indicates the sensor is disconnected or faulty)
  if(realTemp == -127.00) {
    Serial.println("Error: Could not read temperature data");
  } else {
    Serial.print("Temperature is: ");
    Serial.println(realTemp);

    // Get the current timestamp string
    String timeStr = getCurrentTime();

    // Construct the JSON payload string manually
    // Format: {"sensor_id": "ESP32_01", "temp": 25.5, "timestamp": "2024-01-01 12:00:00"}
    String payload = "{\"sensor_id\": \"ESP32_01\", \"temp\": " + String(realTemp) + ", \"timestamp\": \"" + timeStr + "\"}";
    
    Serial.print("Sending payload: ");
    Serial.println(payload);
    
    // Publish the payload to the specified MQTT topic
    client.publish("kidbright/temp", payload.c_str());
  }

  delay(5000); // Wait for 5 seconds before the next reading
}
