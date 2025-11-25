#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// =========================
// CONFIG
// =========================
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

String serverURL = "http://YOUR_SERVER_IP/temperature"; 
// ตัวอย่าง: http://192.168.1.10:3000/temperature

#define ONE_WIRE_BUS 4   // ขาเชื่อม DS18B20

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastSend = 0;
int sendInterval = 5000;  // ส่งทุก 5 วินาที

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);
  delay(1000);

  sensors.begin();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  // รอเชื่อมต่อ WiFi
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// =========================
// LOOP
// =========================
void loop() {
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();

    // อ่านค่าอุณหภูมิ
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC == DEVICE_DISCONNECTED_C) {
      Serial.println("Sensor error!");
      return;
    }

    Serial.print("Temp = ");
    Serial.println(tempC);

    // ส่งข้อมูลขึ้นเซิร์ฟเวอร์
    sendToServer(tempC);
  }
}

// =========================
// ชุดคำสั่งส่งอุณหภูมิแบบ POST
// =========================
void sendToServer(float tempValue) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  HTTPClient http;
  http.begin(serverURL);

  http.addHeader("Content-Type", "application/json");

  // เตรียม JSON payload
  String jsonData = "{";
  jsonData += "\"device_id\":\"esp32-001\",";
  jsonData += "\"temperature\":" + String(tempValue) + ",";
  jsonData += "\"timestamp\":" + String(millis());
  jsonData += "}";

  // ส่ง POST
  int httpResponseCode = http.POST(jsonData);

  Serial.print("Server response: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    Serial.println(http.getString());
  }

  http.end();
}
