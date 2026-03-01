#ifndef VINNY_MOODS_H
#define VINNY_MOODS_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// External references to objects you'll define in your main file
extern Adafruit_SSD1306 display;
extern Servo leftArm;
extern Servo rightArm;

// Master eye drawing function
void drawVinnyEyes(int topEyelid, int bottomEyelid, int lookX);

// Mood functions you can call
void moodHappy();
void moodTired();
void moodSleepy();
void moodSnoring();
void moodStretching();
void moodAlarm();

#endif
