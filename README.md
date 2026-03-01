# ⏰ Expressive Budster

An interactive, character-driven smart alarm clock built with the **Seeed Studio XIAO ESP32C3**. This project brings a standard clock to life using procedural OLED eye animations and coordinated servo-driven arms to express different moods based on the time of day or user interaction.

## ✨ Features
* **Procedural Eye Animations:** Uses geometric masking on an SSD1306 OLED to create smooth, expressive eyes without consuming excess memory with image files.
* **Coordinated Arm Movements:** Two micro-servos act as arms that react alongside the facial expressions.
* **Dynamic Moods:** Transitions seamlessly between states like Happy, Tired, Sleepy, Stretching, and an active Alarm sequence.
* **Custom Enclosure:** Physical housing and mounting brackets designed entirely in Solidworks.

---

## 🛠️ Hardware & CAD Design (Per Module)
* **Microcontroller:** [Seeed Studio XIAO ESP32C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html)
* **Display:** 0.96" I2C OLED Display (SSD1306)
* **Actuators:** 2x Micro Servos (e.g., SG90)
* **Power:** 3.3V supply (via power bank)

### Mechanical Design
The physical enclosure and servo mounts were modeled using **Solidworks**. 
* The CAD files (including native `.SLDPRT`/`.SLDASM` and `.STL` files for 3D printing, which are not included here) can be found in the `/housing` directory of this repository. Previous versions of the arm are still in the repository, with V2 files being the most up to date.
* The Arduino files were .ino and relate to the `/expressions-programV2` and `/brain-program` files in the repository.

---

## ⚙️ Setup and Installation

### 1. Wiring Guide
Ensure your components are wired exactly as follows before connecting power:

| Component | Pin Name | XIAO Pin (Label) | ESP32 GPIO |
| :--- | :--- | :--- | :--- |
| **OLED SDA** | SDA | D4 | GPIO 6 |
| **OLED SCL** | SCL | D5 | GPIO 7 |
| **Left Arm Servo** | Signal | D0 | GPIO 2 |
| **Right Arm Servo** | Signal | D1 | GPIO 3 |
| **All Components** | VCC | 3V3 | - |
| **All Components** | GND | GND | - |

### 2. Software Installation
This project is programmed using the Arduino IDE.
1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Add ESP32 Support: 
   * Go to **File > Preferences**.
   * Add `https://espressif.github.io/arduino-esp32/package_esp32_index.json` to the **Additional Boards Manager URLs**.
   * Go to **Tools > Board > Boards Manager**, search for `esp32` by Espressif Systems, and install it (Version 3.0.0 or higher required).
3. Select your board: Go to **Tools > Board > esp32** and select **XIAO_ESP32C3**.

### 3. Required Libraries
Open the Arduino Library Manager (**Sketch > Include Library > Manage Libraries...**) and install the following:
* `Adafruit_GFX` by Adafruit
* `Adafruit_SSD1306` by Adafruit
* `ESP32Servo` by Kevin Harrington (Standard `Servo.h` will not work on the ESP32 chip).

### 4. Uploading the Code
1. Connect the XIAO ESP32C3 to your computer via USB-C.
2. Select the correct COM port under **Tools > Port**.
3. Open the `expressions-programV2.ino` sketch.
4. Click the **Upload** arrow.

---

## 🤝 Attributions & Acknowledgments
This project was made possible by several excellent open-source tools, libraries, and community inspirations:
* **Adafruit Industries:** For the `Adafruit_GFX` and `Adafruit_SSD1306` libraries that drive the OLED display.
* **Kevin Harrington:** For the `ESP32Servo` library, allowing standard PWM servo control on the ESP32 architecture.
* **Espressif Systems & Seeed Studio:** For the ESP32 core board definitions and the XIAO hardware.
* **Design Inspiration:** The procedural masking logic for the eye expressions was heavily inspired by the "Vinny" robot eye designs and various open-source animated OLED eye projects found across the maker community. 
* **Solidworks:** Used for all 3D mechanical design, part modeling, and assembly testing.
* **AI & LLM policy:** Used to generate only code and execute ideas we had as a team.
