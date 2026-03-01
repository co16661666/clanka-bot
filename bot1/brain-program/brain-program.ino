#include "arduino_secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <time.h>

// ============================================================
// Configurable Pin Constants (modify these for your wiring)
// ============================================================
const int POT_PIN         = 34;   // Potentiometer analog input pin
const int TOUCH_PIN_A     = 4;    // Touch pin A (confirm / select)
const int TOUCH_PIN_B     = 15;   // Touch pin B (back / cancel)
const int TOUCH_THRESHOLD = 40;   // Touch read below this = touched
const int POT_THRESHOLD   = 100;  // Pot reading above this = signal

// ============================================================
// State Machine
// ============================================================
enum State {
  S0_IDLE,
  S1_MENU,
  S2_DISPLAY_MSG,
  S3_SEND_MSG,
  S4_SET_ALARM,
  S5_PUBLISH
};

State currentState = S0_IDLE;
int   menuSelection = 0;   // 0 = Send Message, 1 = Set Alarm
int   msgSelection  = 0;   // index into combined messages list

// ============================================================
// Preset Messages & Emotes
// ============================================================
const char* PRESET_MESSAGES[] = {
  "drink some water!",
  "i finished my hw! have you?"
};
const int NUM_PRESET_MESSAGES = 2;

const char* EMOTE_STRINGS[] = {
  "happy",
  "tired",
  "sleepy",
  "snoring",
  "stretching"
};
const int NUM_EMOTES = 5;

// Total selectable items in the send-message menu
const int NUM_SENDABLE = NUM_PRESET_MESSAGES + NUM_EMOTES;

// ============================================================
// Incoming Message Buffer
// ============================================================
bool   newMessageFlag      = false;
String lastReceivedMessage = "";
String lastReceivedFrom    = "";

// ============================================================
// Network & MQTT
// ============================================================
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

// This bot's identity
String nickname              = "bot1";
String aws_iot_publish_topic = "esp32/" + nickname;

// Subscribe to the OTHER bot (two-bot system)
String aws_iot_receive_topic = "esp32/bot2";

// ============================================================
// Emote Placeholder Functions (implement later)
// ============================================================
void emoteHappy()      { Serial.println("[EMOTE] happy — not yet implemented"); }
void emoteTired()      { Serial.println("[EMOTE] tired — not yet implemented"); }
void emoteSleepy()     { Serial.println("[EMOTE] sleepy — not yet implemented"); }
void emoteSnoring()    { Serial.println("[EMOTE] snoring — not yet implemented"); }
void emoteStretching() { Serial.println("[EMOTE] stretching — not yet implemented"); }

// Dispatch emote by index (0-4)
void playEmote(int index) {
  switch (index) {
    case 0: emoteHappy();      break;
    case 1: emoteTired();      break;
    case 2: emoteSleepy();     break;
    case 3: emoteSnoring();    break;
    case 4: emoteStretching(); break;
  }
}

// Check if a message string is a special emote
bool isEmote(const char* msg) {
  for (int i = 0; i < NUM_EMOTES; i++) {
    if (strcmp(msg, EMOTE_STRINGS[i]) == 0) return true;
  }
  return false;
}

// Play emote by name (used when receiving messages)
void playEmoteByName(const char* name) {
  for (int i = 0; i < NUM_EMOTES; i++) {
    if (strcmp(name, EMOTE_STRINGS[i]) == 0) {
      playEmote(i);
      return;
    }
  }
}

// ============================================================
// AWS / MQTT Functions
// ============================================================
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

  // Connect to the MQTT broker on the AWS endpoint
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
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

  // Subscribe to the other bot's topic
  client.subscribe(aws_iot_receive_topic);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(int messageIndex)
{
  // Determine the actual message string
  const char* msgToSend;
  if (messageIndex < NUM_PRESET_MESSAGES) {
    msgToSend = PRESET_MESSAGES[messageIndex];
  } else {
    msgToSend = EMOTE_STRINGS[messageIndex - NUM_PRESET_MESSAGES];
  }

  StaticJsonDocument<200> doc;
  doc["from"]    = nickname;
  doc["message"] = msgToSend;

  // Add timestamp
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char timeBuf[30];
  strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S UTC", &timeinfo);
  doc["time"] = timeBuf;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  if (client.publish(aws_iot_publish_topic, jsonBuffer)) {
    Serial.println("Published: " + String(msgToSend));
  } else {
    Serial.println("Publish Failed");
  }
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* msg = doc["message"];
  if (msg) {
    lastReceivedMessage = String(msg);
    lastReceivedFrom    = topic;
    newMessageFlag      = true;
  }
}

// ============================================================
// Input Helper
// ============================================================
bool readInputSignal(int &potValue, bool &touchA, bool &touchB) {
  potValue = analogRead(POT_PIN);
  touchA   = (touchRead(TOUCH_PIN_A) < TOUCH_THRESHOLD);
  touchB   = (touchRead(TOUCH_PIN_B) < TOUCH_THRESHOLD);
  return (potValue > POT_THRESHOLD) || touchA || touchB;
}

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(9600);
  connectAWS();
}

// ============================================================
// Main Loop — State Machine
// ============================================================
void loop() {
  client.loop();

  int  potValue;
  bool touchA, touchB;
  bool hasSignal = readInputSignal(potValue, touchA, touchB);

  switch (currentState) {

    // ---- S0: IDLE — wait for input or incoming message ----
    case S0_IDLE:
      if (hasSignal) {
        currentState  = S1_MENU;
        menuSelection = 0;
        Serial.println("=== MENU ===");
        Serial.println("0: Send Message");
        Serial.println("1: Set Alarm");
      } else if (newMessageFlag) {
        currentState = S2_DISPLAY_MSG;
      }
      break;

    // ---- S1: MENU — choose Send Message or Set Alarm ----
    case S1_MENU:
      menuSelection = (potValue > (4095 / 2)) ? 1 : 0;

      if (touchA) {  // confirm
        if (menuSelection == 0) {
          currentState = S3_SEND_MSG;
          msgSelection = 0;
          Serial.println("=== SEND MESSAGE ===");
          Serial.println("-- Messages --");
          for (int i = 0; i < NUM_PRESET_MESSAGES; i++) {
            Serial.println(String(i) + ": " + PRESET_MESSAGES[i]);
          }
          Serial.println("-- Emotes --");
          for (int i = 0; i < NUM_EMOTES; i++) {
            Serial.println(String(i + NUM_PRESET_MESSAGES) + ": [" + EMOTE_STRINGS[i] + "]");
          }
        } else {
          currentState = S4_SET_ALARM;
          Serial.println("=== SET ALARM (placeholder) ===");
        }
        delay(300); // debounce
      }

      if (touchB) {  // back
        currentState = S0_IDLE;
        delay(300);
      }
      break;

    // ---- S2: DISPLAY incoming message ----
    case S2_DISPLAY_MSG:
      Serial.println(">> New message from " + lastReceivedFrom + ": " + lastReceivedMessage);

      // If the received message is a special emote, trigger the emote
      if (isEmote(lastReceivedMessage.c_str())) {
        Serial.println(">> Playing emote: " + lastReceivedMessage);
        playEmoteByName(lastReceivedMessage.c_str());
      }

      newMessageFlag = false;
      currentState   = S0_IDLE;
      delay(2000); // display time
      break;

    // ---- S3: SEND MESSAGE — pick from preset messages + emotes ----
    case S3_SEND_MSG:
      msgSelection = map(potValue, 0, 4095, 0, NUM_SENDABLE - 1);
      msgSelection = constrain(msgSelection, 0, NUM_SENDABLE - 1);

      if (touchA) {  // confirm selection
        currentState = S5_PUBLISH;
        delay(300);
      }
      if (touchB) {  // back to menu
        currentState = S1_MENU;
        delay(300);
      }
      break;

    // ---- S4: SET ALARM — placeholder ----
    case S4_SET_ALARM:
      Serial.println("Alarm feature not yet implemented.");
      if (touchA || touchB) {
        currentState = S1_MENU;
        delay(300);
      }
      break;

    // ---- S5: PUBLISH the selected message ----
    case S5_PUBLISH: {
      // Check if the selection is an emote (index >= NUM_PRESET_MESSAGES)
      bool selectedIsEmote = (msgSelection >= NUM_PRESET_MESSAGES);

      if (selectedIsEmote) {
        int emoteIndex = msgSelection - NUM_PRESET_MESSAGES;
        Serial.println("Sending emote: " + String(EMOTE_STRINGS[emoteIndex]));
        // Play our own emote locally as well
        playEmote(emoteIndex);
      }

      // Publish to MQTT (works for both messages and emotes)
      publishMessage(msgSelection);

      Serial.println("Message sent! Returning to idle.");
      currentState = S0_IDLE;
      delay(1000);
      break;
    }
  }

  delay(100); // main loop tick
}