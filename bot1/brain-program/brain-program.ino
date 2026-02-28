#include "arduino_secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <time.h>

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

String nickname = "bot1";
String aws_iot_publish_topic = "esp32/" + nickname;
String aws_iot_receive_topic = "esp32/john";

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Set time via NTP for UTC (UTC+0)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("\nWaiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");
  
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    
    // Print diagnostic information
    Serial.println();
    Serial.print("MQTT Error Code: ");
    Serial.println(client.lastError());
    Serial.print("MQTT Return Code: ");
    Serial.println(client.returnCode());
    
    delay(1000);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(aws_iot_receive_topic);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char timeBuffer[30];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S UTC", &timeinfo);

  StaticJsonDocument<200> doc;
  doc["time"] = timeBuffer;
  doc["sensor_a0"] = analogRead(0);
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  if (client.publish(aws_iot_publish_topic, jsonBuffer)) {
    Serial.println("Publish Success");
  } else {
    Serial.println("Publish Failed");
  }
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
}

void setup() {
  Serial.begin(9600);
  connectAWS();
}

void loop() {
  publishMessage();
  client.loop();
  delay(1000);
}