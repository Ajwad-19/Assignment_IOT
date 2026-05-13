#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED PINS =================
#define OLED_SDA 21
#define OLED_SCL 22

// ================= OLED SETTINGS =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool oledOK = false;

// ================= SENSOR / ACTUATOR PINS =================
#define LDR_PIN 34
#define LED_PIN 25

#define MQ_PIN 35
#define BUZZER_PIN 26

#define FAN_RELAY_PIN 27

#define DHT_PIN 32
#define DHT_TYPE DHT11   // Change to DHT22 if your sensor is DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// ================= LDR SETTINGS =================
int darkThresholdPercent = 30;

// Your LDR showed 4095 when dark, so this must be false
bool brightValueIsHigh = false;

// ================= SMOKE SETTINGS =================
// Adjust these after checking normal air value
int safeLimit = 60;
int dangerLimit = 80;

// ================= BUZZER / RELAY SETTINGS =================
// Most buzzer modules: HIGH = ON, LOW = OFF
bool activeHighBuzzer = true;

// Relay modules are sometimes active LOW.
// If fan works opposite, change this to true.
bool activeLowFanRelay = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  setBuzzer(false);
  setFan(false);

  analogReadResolution(12);

  // Start DHT sensor
  dht.begin();

  // Start OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    oledOK = true;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("Smart Room System");
    display.println("Starting...");
    display.display();

    Serial.println("OLED detected.");
  } else {
    oledOK = false;
    Serial.println("OLED not detected. System continues without OLED.");
  }

  delay(2000);

  Serial.println("System Started");
}

void loop() {
  // ================= LDR READING =================
  int ldrRaw = analogRead(LDR_PIN);

  int brightnessPercent;

  if (brightValueIsHigh == true) {
    brightnessPercent = map(ldrRaw, 0, 4095, 0, 100);
  } else {
    brightnessPercent = map(ldrRaw, 0, 4095, 100, 0);
  }

  brightnessPercent = constrain(brightnessPercent, 0, 100);

  bool isDark = brightnessPercent < darkThresholdPercent;

  String lightStatus;

  if (isDark) {
    digitalWrite(LED_PIN, HIGH);   // LED ON when dark
    lightStatus = "ON";
  } else {
    digitalWrite(LED_PIN, LOW);    // LED OFF when bright
    lightStatus = "OFF";
  }

  // ================= MQ SMOKE READING =================
  int mqRaw = analogRead(MQ_PIN);

  int smokePercent = map(mqRaw, 0, 4095, 0, 100);
  smokePercent = constrain(smokePercent, 0, 100);

  String airStatus;
  String fanStatus;

  if (smokePercent < safeLimit) {
    airStatus = "SAFE";
    setBuzzer(false);
    setFan(false);
    fanStatus = "OFF";
  }
  else if (smokePercent < dangerLimit) {
    airStatus = "MODERATE";
    setBuzzer(false);
    setFan(false);
    fanStatus = "OFF";
  }
  else {
    airStatus = "DANGER";
    setBuzzer(true);    // Buzzer ON only when danger
    setFan(true);       // Fan ON only when danger
    fanStatus = "ON";
  }

  // ================= DHT TEMPERATURE & HUMIDITY =================
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  bool dhtOK = true;

  if (isnan(temperature) || isnan(humidity)) {
    dhtOK = false;
    Serial.println("Failed to read from DHT sensor!");
  }

  // ================= OLED DISPLAY =================
  if (oledOK == true) {
    showOLED(brightnessPercent, lightStatus, smokePercent, airStatus, fanStatus, temperature, humidity, dhtOK);
  }

  // ================= SERIAL MONITOR OUTPUT =================
  Serial.print("LDR Raw: ");
  Serial.print(ldrRaw);

  Serial.print(" | Brightness: ");
  Serial.print(brightnessPercent);
  Serial.print("%");

  Serial.print(" | LED: ");
  Serial.print(lightStatus);

  Serial.print(" | MQ Raw: ");
  Serial.print(mqRaw);

  Serial.print(" | Smoke: ");
  Serial.print(smokePercent);
  Serial.print("%");

  Serial.print(" | Air Status: ");
  Serial.print(airStatus);

  Serial.print(" | Buzzer: ");
  if (airStatus == "DANGER") {
    Serial.print("ON");
  } else {
    Serial.print("OFF");
  }

  Serial.print(" | Fan: ");
  Serial.print(fanStatus);

  if (dhtOK == true) {
    Serial.print(" | Temp: ");
    Serial.print(temperature);
    Serial.print(" C");

    Serial.print(" | Humidity: ");
    Serial.print(humidity);
    Serial.print("%");
  } else {
    Serial.print(" | DHT: ERROR");
  }

  Serial.println();
  Serial.println("--------------------------------");

  delay(2000);
}

// ================= BUZZER CONTROL FUNCTION =================
void setBuzzer(bool state) {
  if (activeHighBuzzer == true) {
    digitalWrite(BUZZER_PIN, state ? HIGH : LOW);
  } else {
    digitalWrite(BUZZER_PIN, state ? LOW : HIGH);
  }
}

// ================= FAN RELAY CONTROL FUNCTION =================
void setFan(bool state) {
  if (activeLowFanRelay == true) {
    digitalWrite(FAN_RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(FAN_RELAY_PIN, state ? HIGH : LOW);
  }
}

// ================= OLED DISPLAY FUNCTION =================
void showOLED(
  int brightnessPercent,
  String lightStatus,
  int smokePercent,
  String airStatus,
  String fanStatus,
  float temperature,
  float humidity,
  bool dhtOK
) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart Room System");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.print("B:");
  display.print(brightnessPercent);
  display.print("% LED:");
  display.println(lightStatus);

  display.setCursor(0, 23);
  display.print("Smoke:");
  display.print(smokePercent);
  display.print("% ");
  display.println(airStatus);

  display.setCursor(0, 33);
  display.print("Fan:");
  display.print(fanStatus);
  display.print(" Buzz:");
  if (airStatus == "DANGER") {
    display.println("ON");
  } else {
    display.println("OFF");
  }

  display.setCursor(0, 43);
  if (dhtOK == true) {
    display.print("Temp:");
    display.print(temperature, 1);
    display.println(" C");

    display.setCursor(0, 53);
    display.print("Humid:");
    display.print(humidity, 1);
    display.println("%");
  } else {
    display.println("DHT Error");
  }

  display.display();
}