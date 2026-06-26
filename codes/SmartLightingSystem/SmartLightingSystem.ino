/*************************************************************
  BLYNK SETTINGS
*************************************************************/
#define BLYNK_TEMPLATE_ID   "TMPL6uf9E0kmK"
#define BLYNK_TEMPLATE_NAME "Smart Room System"
#define BLYNK_AUTH_TOKEN    "QNUGwVi35fPH5frUQGR0xW569otnxc5y"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>

/*************************************************************
  WIFI SETTINGS
*************************************************************/
char ssid[] = "jed";
char pass[] = "YOUR_WIFI_PASSWORD";   // <-- put your WiFi password here

BlynkTimer timer;

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

// ================= SENSOR PINS =================
#define PIR_PIN 17
#define DHT_PIN 19
#define LDR_PIN 34
#define MQ_PIN  35

// ================= RELAY OUTPUT PINS =================
#define FAN_RELAY_PIN    26
#define BUZZER_RELAY_PIN 25
#define LED_RELAY_PIN    27

// ================= SERVO PIN =================
#define SERVO_PIN 16
Servo myservo;

// ================= DHT SETTINGS =================
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ================= LDR SETTINGS =================
int darkThresholdPercent = 30;
// Your LDR showed 4095 when dark, so false is correct
bool brightValueIsHigh = false;

// ================= PIR SETTINGS =================
// PIR OUT goes HIGH when motion is detected
bool pirActiveHigh = true;

// ================= SERVO SETTINGS =================
int servoClosedAngle = 0;
int servoOpenAngle   = 90;     // change to 180 if your door needs a wider swing

// After motion stops, wait this long before closing the servo
unsigned long servoCloseDelay = 3000;   // ms
unsigned long noMotionStartTime = 0;

// Startup protection: do not attach/move servo until first motion
bool servoAttached       = false;
bool servoIsOpen         = false;
bool noMotionTimerStarted = false;

// ================= SMOKE SETTINGS =================
int safeLimit   = 60;   // below 60%      = SAFE  (buzzer turns on at/above this)
int dangerLimit = 80;   // 60% to 79%     = MODERATE
                        // 80% and above  = DANGER (fan trigger)

// ================= TEMPERATURE FAN SETTINGS =================
float fanTemperatureLimit = 40.0;   // Fan ON when temperature >= 40C

// ================= RELAY SETTINGS =================
// CW-020 relay modules are usually active LOW (LOW = ON, HIGH = OFF)
bool activeLowLEDRelay    = true;
bool activeLowBuzzerRelay = true;
bool activeLowFanRelay    = true;

// ================= SENSOR VALUES =================
int ldrRaw = 0;
int brightnessPercent = 0;
bool isDark = false;

int pirValue = 0;
bool personDetected = false;
String pirStatus = "NO";

String servoStatus = "WAIT";   // WAIT until first motion (startup protection)
String ledRelayStatus = "OFF";

int mqRaw = 0;
int smokePercent = 0;
String airStatus = "SAFE";

String fanRelayStatus = "OFF";
String buzzerRelayStatus = "OFF";
String fanReason = "OFF";      // OFF / SMOKE / TEMP / SMOKE+TEMP

float temperature = 0;
float humidity = 0;
bool dhtOK = false;

unsigned long lastDHTRead = 0;
unsigned long dhtInterval = 2000;

// ================= BLYNK TIMING =================
unsigned long lastBlynkConnectAttempt = 0;
unsigned long blynkConnectInterval = 10000;

/*************************************************************
  SETUP
*************************************************************/
void setup() {
  Serial.begin(115200);

  // INPUT_PULLDOWN keeps the PIR pin from floating when idle
  pinMode(PIR_PIN, INPUT_PULLDOWN);

  pinMode(LED_RELAY_PIN, OUTPUT);
  pinMode(BUZZER_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);

  // Start all relays OFF
  setLEDRelay(false);
  setBuzzerRelay(false);
  setFanRelay(false);

  analogReadResolution(12);

  dht.begin();

  // NOTE: Servo is intentionally NOT attached here.
  // It attaches on the first motion event so it does not
  // twitch during power-up.

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

  // Start WiFi and Blynk (non-blocking)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Blynk.config(BLYNK_AUTH_TOKEN);

  // Push data to Blynk every 1 second
  timer.setInterval(1000L, sendToBlynk);

  delay(2000);

  Serial.println("System Started");
}

/*************************************************************
  LOOP
*************************************************************/
void loop() {
  handleBlynkConnection();

  if (Blynk.connected()) {
    Blynk.run();
  }

  timer.run();

  readLDR();
  readPIRAndControlServo();
  controlLEDRelay();
  readMQAndControlSmokeSystem();
  readDHT();
  controlFanBySmokeOrTemperature();

  if (oledOK == true) {
    showOLED();
  }

  printSerialMonitor();

  delay(300);
}

/*************************************************************
  SENSOR AND CONTROL FUNCTIONS
*************************************************************/

void readLDR() {
  ldrRaw = analogRead(LDR_PIN);

  if (brightValueIsHigh == true) {
    brightnessPercent = map(ldrRaw, 0, 4095, 0, 100);
  } else {
    brightnessPercent = map(ldrRaw, 0, 4095, 100, 0);
  }

  brightnessPercent = constrain(brightnessPercent, 0, 100);
  isDark = brightnessPercent < darkThresholdPercent;
}

void readPIRAndControlServo() {
  pirValue = digitalRead(PIR_PIN);

  if (pirActiveHigh == true) {
    personDetected = (pirValue == HIGH);
  } else {
    personDetected = (pirValue == LOW);
  }

  pirStatus = personDetected ? "YES" : "NO";

  // Presence-based behavior:
  //  - motion present  -> attach (first time) and open, keep open
  //  - motion gone      -> after servoCloseDelay, close
  if (personDetected == true) {
    noMotionTimerStarted = false;

    if (servoAttached == false) {
      myservo.setPeriodHertz(50);
      myservo.attach(SERVO_PIN, 500, 2400);
      servoAttached = true;
    }

    if (servoIsOpen == false) {
      myservo.write(servoOpenAngle);
      servoIsOpen = true;
    }
  } else {
    if (servoIsOpen == true) {
      if (noMotionTimerStarted == false) {
        noMotionStartTime = millis();
        noMotionTimerStarted = true;
      }

      if (millis() - noMotionStartTime >= servoCloseDelay) {
        myservo.write(servoClosedAngle);
        servoIsOpen = false;
        noMotionTimerStarted = false;
      }
    }
  }

  if (servoAttached == false) {
    servoStatus = "WAIT";
  } else {
    servoStatus = servoIsOpen ? String(servoOpenAngle) : String(servoClosedAngle);
  }
}

void controlLEDRelay() {
  // LED relay ON when the room is dark
  if (isDark) {
    setLEDRelay(true);
    ledRelayStatus = "ON";
  } else {
    setLEDRelay(false);
    ledRelayStatus = "OFF";
  }
}

void readMQAndControlSmokeSystem() {
  mqRaw = analogRead(MQ_PIN);

  smokePercent = map(mqRaw, 0, 4095, 0, 100);
  smokePercent = constrain(smokePercent, 0, 100);

  if (smokePercent < safeLimit) {
    airStatus = "SAFE";
  } else if (smokePercent < dangerLimit) {
    airStatus = "MODERATE";
  } else {
    airStatus = "DANGER";
  }

  // Buzzer ON as soon as smoke leaves the SAFE band
  bool smokeDetected = smokePercent >= safeLimit;
  setBuzzerRelay(smokeDetected);
  buzzerRelayStatus = smokeDetected ? "ON" : "OFF";
}

void readDHT() {
  if (millis() - lastDHTRead >= dhtInterval) {
    lastDHTRead = millis();

    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      dhtOK = false;
      Serial.println("Failed to read from DHT sensor!");
    } else {
      dhtOK = true;
    }
  }
}

void controlFanBySmokeOrTemperature() {
  bool smokeDangerDetected     = smokePercent >= dangerLimit;
  bool highTemperatureDetected = (dhtOK == true && temperature >= fanTemperatureLimit);

  bool fanShouldTurnOn = smokeDangerDetected || highTemperatureDetected;

  setFanRelay(fanShouldTurnOn);
  fanRelayStatus = fanShouldTurnOn ? "ON" : "OFF";

  if (smokeDangerDetected && highTemperatureDetected) {
    fanReason = "SMOKE+TEMP";
  } else if (smokeDangerDetected) {
    fanReason = "SMOKE";
  } else if (highTemperatureDetected) {
    fanReason = "TEMP";
  } else {
    fanReason = "OFF";
  }
}

/*************************************************************
  BLYNK FUNCTIONS
*************************************************************/

void handleBlynkConnection() {
  if (WiFi.status() == WL_CONNECTED && !Blynk.connected()) {
    if (millis() - lastBlynkConnectAttempt >= blynkConnectInterval) {
      lastBlynkConnectAttempt = millis();
      Serial.println("Trying to connect to Blynk...");
      Blynk.connect(3000);
    }
  }
}

void sendToBlynk() {
  if (!Blynk.connected()) {
    return;
  }

  Blynk.virtualWrite(V0, brightnessPercent);
  Blynk.virtualWrite(V1, pirStatus);
  Blynk.virtualWrite(V2, servoStatus);
  Blynk.virtualWrite(V3, ledRelayStatus);
  Blynk.virtualWrite(V4, smokePercent);
  Blynk.virtualWrite(V5, airStatus);
  Blynk.virtualWrite(V6, fanRelayStatus);
  Blynk.virtualWrite(V7, buzzerRelayStatus);

  if (dhtOK == true) {
    Blynk.virtualWrite(V8, temperature);
    Blynk.virtualWrite(V9, humidity);
  }

  // New: fan reason (add a V10 datastream in Blynk if you want this on the dashboard)
  Blynk.virtualWrite(V10, fanReason);
}

/*************************************************************
  RELAY CONTROL FUNCTIONS
*************************************************************/

void setLEDRelay(bool state) {
  if (activeLowLEDRelay == true) {
    digitalWrite(LED_RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(LED_RELAY_PIN, state ? HIGH : LOW);
  }
}

void setBuzzerRelay(bool state) {
  if (activeLowBuzzerRelay == true) {
    digitalWrite(BUZZER_RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(BUZZER_RELAY_PIN, state ? HIGH : LOW);
  }
}

void setFanRelay(bool state) {
  if (activeLowFanRelay == true) {
    digitalWrite(FAN_RELAY_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(FAN_RELAY_PIN, state ? HIGH : LOW);
  }
}

/*************************************************************
  OLED DISPLAY FUNCTION
*************************************************************/

void showOLED() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart Room System");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.print("B:");
  display.print(brightnessPercent);
  display.print("% PIR:");
  display.println(pirStatus);

  display.setCursor(0, 23);
  display.print("Servo:");
  display.print(servoStatus);
  display.print(" LED:");
  display.println(ledRelayStatus);

  display.setCursor(0, 33);
  display.print("Smoke:");
  display.print(smokePercent);
  display.print("% ");
  display.println(airStatus);

  display.setCursor(0, 43);
  display.print("Fan:");
  display.print(fanRelayStatus);
  display.print(" ");
  display.println(fanReason);

  display.setCursor(0, 53);
  if (dhtOK == true) {
    display.print("T:");
    display.print(temperature, 1);
    display.print("C H:");
    display.print(humidity, 0);
    display.print("%");
  } else {
    display.print("DHT Error");
  }

  display.display();
}

/*************************************************************
  SERIAL MONITOR FUNCTION
*************************************************************/

void printSerialMonitor() {
  Serial.print("WiFi: ");
  Serial.print(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED");

  Serial.print(" | Blynk: ");
  Serial.print(Blynk.connected() ? "CONNECTED" : "DISCONNECTED");

  Serial.print(" | LDR Raw: ");
  Serial.print(ldrRaw);

  Serial.print(" | Brightness: ");
  Serial.print(brightnessPercent);
  Serial.print("%");

  Serial.print(" | PIR: ");
  Serial.print(pirStatus);

  Serial.print(" | Servo: ");
  Serial.print(servoStatus);

  Serial.print(" | LED Relay: ");
  Serial.print(ledRelayStatus);

  Serial.print(" | MQ Raw: ");
  Serial.print(mqRaw);

  Serial.print(" | Smoke: ");
  Serial.print(smokePercent);
  Serial.print("%");

  Serial.print(" | Air: ");
  Serial.print(airStatus);

  Serial.print(" | Fan Relay: ");
  Serial.print(fanRelayStatus);

  Serial.print(" | Fan Reason: ");
  Serial.print(fanReason);

  Serial.print(" | Buzzer Relay: ");
  Serial.print(buzzerRelayStatus);

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
}
