#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED pins
#define OLED_SDA 21
#define OLED_SCL 22

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin connections
#define MQ_PIN 35
#define BUZZER_PIN 26

// Thresholds
int safeLimit = 30;      // Below 30% = SAFE
int dangerLimit = 60;    // 30% to 59% = MODERATE, 60% and above = DANGER

// Most buzzer modules: HIGH = ON, LOW = OFF
bool activeHighBuzzer = true;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  setBuzzer(false);   // Buzzer OFF at start

  analogReadResolution(12); // ESP32 ADC: 0 to 4095

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not detected. Check wiring or OLED address.");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Smoke Detector");
  display.println("System Starting...");
  display.display();

  delay(3000);
}

void loop() {
  int mqRaw = analogRead(MQ_PIN);

  int smokePercent = map(mqRaw, 0, 4095, 0, 100);
  smokePercent = constrain(smokePercent, 0, 100);

  String airStatus;

  if (smokePercent < safeLimit) {
    airStatus = "SAFE";
    setBuzzer(false);
  }
  else if (smokePercent < dangerLimit) {
    airStatus = "MODERATE";
    setBuzzer(false);
  }
  else {
    airStatus = "DANGER";
    setBuzzer(true);   // Buzzer ON only here
  }

  showOLED(smokePercent, airStatus);

  Serial.print("MQ Raw: ");
  Serial.print(mqRaw);
  Serial.print(" | Smoke Level: ");
  Serial.print(smokePercent);
  Serial.print("% | Status: ");
  Serial.print(airStatus);
  Serial.print(" | Buzzer: ");

  if (airStatus == "DANGER") {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  delay(1000);
}

void setBuzzer(bool state) {
  if (activeHighBuzzer == true) {
    digitalWrite(BUZZER_PIN, state ? HIGH : LOW);
  } else {
    digitalWrite(BUZZER_PIN, state ? LOW : HIGH);
  }
}

void showOLED(int smokePercent, String airStatus) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Air Quality Status");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("Smoke: ");
  display.print(smokePercent);
  display.println("%");

  display.setCursor(0, 32);
  display.print("Status:");

  display.setTextSize(2);
  display.setCursor(0, 44);
  display.println(airStatus);

  display.display();
}