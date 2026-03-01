v#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h> // Make sure to install "ESP32Servo" in the Library Manager!

// --- OLED SETTINGS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// XIAO ESP32C3 I2C Pins
#define I2C_SDA 6 
#define I2C_SCL 7 

// --- SERVO SETTINGS ---
Servo leftArm;
Servo rightArm;

// XIAO ESP32C3 Pins for Servos (D0 and D1)
const int leftArmPin = 2;  // Physical GPIO 2 corresponds to D0
const int rightArmPin = 3; // Physical GPIO 3 corresponds to D1

// --- MASTER EYE DRAWING FUNCTION ---
void drawVinnyEyes(int topEyelid, int bottomEyelid, int lookX) {
  display.clearDisplay();
  
  int eyeW = 24;
  int eyeH = 40;
  int leftX = 26;
  int rightX = 78; 
  int baseY = 12;

  // Draw the wide-open white eyes, shifted by lookX
  display.fillRoundRect(leftX + lookX, baseY, eyeW, eyeH, 8, SSD1306_WHITE);
  display.fillRoundRect(rightX + lookX, baseY, eyeW, eyeH, 8, SSD1306_WHITE);

  // Draw the Top Eyelid (Black box covering the top)
  if (topEyelid > 0) {
    display.fillRect(0, 0, 128, baseY + topEyelid, SSD1306_BLACK);
  }

  // Draw the Bottom Eyelid (Black box covering the bottom)
  if (bottomEyelid > 0) {
    display.fillRect(0, baseY + eyeH - bottomEyelid, 128, bottomEyelid + 15, SSD1306_BLACK);
  }

  display.display();
}

// ==========================================
// --- COMBINED MOOD FUNCTIONS ---
// ==========================================

void moodHappy() {
  drawVinnyEyes(0, 20, 0); // Joyful squint
  leftArm.write(90);       // Arms neutral
  rightArm.write(90);
}

void moodTired() {
  drawVinnyEyes(20, 0, 0); // Heavy top eyelids
  leftArm.write(45);       // Arms drooping down
  rightArm.write(135); 
}

void moodSleepy() {
  drawVinnyEyes(30, 0, 0); // Almost closed eyes
  leftArm.write(10);       // Arms fully resting
  rightArm.write(170); 
}

void moodSnoring() {
  drawVinnyEyes(18, 18, 0); // Squished flat lines
  
  // Add floating 'Zzz'
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(55, 0);
  display.print("Z");
  display.setCursor(65, 8);
  display.print("z");
  display.display();
  
  leftArm.write(10);       // Arms resting
  rightArm.write(170);
}

void moodStretching() {
  // Eyes look left, arms go up
  drawVinnyEyes(0, 0, -8); 
  leftArm.write(150);  
  rightArm.write(30);  
  delay(600);
  
  // Eyes look right, arms wiggle
  drawVinnyEyes(0, 0, 8);  
  leftArm.write(120);
  rightArm.write(60);
  delay(300);
  
  // Eyes center, stretch finishes
  drawVinnyEyes(0, 0, 0);
  leftArm.write(150);
  rightArm.write(30);
  delay(600);
}

void moodAlarm() {
  // Wide open eyes, frantic arms
  drawVinnyEyes(0, 0, 0);
  leftArm.write(180);
  rightArm.write(0);
  
  display.invertDisplay(true); // Flash screen white
  delay(150);
  
  leftArm.write(0);
  rightArm.write(180);
  
  display.invertDisplay(false); // Flash screen back to normal
  delay(150);
}

// ==========================================
// --- REQUIRED ARDUINO FUNCTIONS ---
// ==========================================

void setup() {
  Serial.begin(115200);

  // 1. Setup Servos
  // We use ESP32Servo library standard attach
  leftArm.attach(leftArmPin);
  rightArm.attach(rightArmPin);

  // 2. Setup OLED with custom XIAO pins
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Loop forever if display fails
  }
  
  display.clearDisplay();
  display.display();
}

void loop() {
  // A simple test sequence to cycle through all your moods!
  
  moodHappy();
  delay(3000);
  
  moodTired();
  delay(3000);
  
  moodSleepy();
  delay(3000);
  
  moodSnoring();
  delay(3000);
  
  moodStretching();
  delay(1000);
  
  // Flash the alarm a few times
  for(int i = 0; i < 10; i++) {
    moodAlarm();
  }
}
