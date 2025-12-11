#include <WiFi.h>
#include <PubSubClient.h>

// --- 1. Set up your WIFI to use (2.4 GHz) ---
const char* ssid = "...";        // Enter WIFI username
const char* password = "..."; // Enter WIFI password

// --- 2. Set up  Server AWS EC2 ---
// Look at AWS in "Public IPv4 address"
const char* mqtt_server = "...";  // Enter IP of EC2 Example "13.214.25.178"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  // Connect WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Set up MQTT
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create random Client ID 
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

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- 3. Mock Temp Data ---
  float mockTemp = random(2000, 4000) / 100.0; // สุ่มเลข 20.00 ถึง 40.00
  
  // Create JSON String
  String payload = "{\"sensor_id\": \"ESP32_01\", \"temp\": " + String(mockTemp) + "}";
  
  Serial.print("Sending payload: ");
  Serial.println(payload);
  
  // Send Data to 'kidbright/temp'
  client.publish("kidbright/temp", payload.c_str());

  // Send every 5 sec
  delay(5000);
}
