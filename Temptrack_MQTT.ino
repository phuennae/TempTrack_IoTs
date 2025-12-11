#include <WiFi.h>
#include <PubSubClient.h>

// --- 1. ตั้งค่า WiFi บ้านคุณ ---
const char* ssid = "...";        // ใส่ชื่อ WiFi
const char* password = "..."; // ใส่รหัส WiFi

// --- 2. ตั้งค่า Server AWS EC2 ---
// ไปดูที่หน้าเว็บ AWS ตรงช่อง "Public IPv4 address"
const char* mqtt_server = "...";  // ใส่ IP ของ EC2 เช่น "13.214.25.178"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // ตั้งค่า MQTT
  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // สร้างชื่อ Client ID แบบสุ่มจะได้ไม่ซ้ำ
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

  // --- 3. จำลองค่าอุณหภูมิ (Mock Data) ---
  float mockTemp = random(2000, 4000) / 100.0; // สุ่มเลข 20.00 ถึง 40.00
  
  // สร้าง JSON String
  String payload = "{\"sensor_id\": \"ESP32_01\", \"temp\": " + String(mockTemp) + "}";
  
  Serial.print("Sending payload: ");
  Serial.println(payload);
  
  // ส่งข้อมูลไปที่หัวข้อ 'kidbright/temp'
  client.publish("kidbright/temp", payload.c_str());

  // ส่งทุกๆ 5 วินาที
  delay(5000);
}
