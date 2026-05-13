// ESP32 LDR + Light Test
// When dark, light turns ON
// Brightness shown in percentage

#define LDR_PIN 34
#define LIGHT_PIN 25

// Adjust this percentage after testing
int darkThresholdPercent = 30;

// false = HIGH turns light ON
// true  = LOW turns light ON, common for relay modules
bool activeLowRelay = false;

// Set this based on your LDR module behavior
// true  = raw value higher when brighter
// false = raw value lower when brighter
bool brightValueIsHigh = true;

void setup() {
  Serial.begin(115200);

  pinMode(LIGHT_PIN, OUTPUT);
  setLight(false);

  analogReadResolution(12); // ESP32 ADC: 0 to 4095

  Serial.println("LDR Brightness Percentage Test Started");
}

void loop() {
  int ldrRaw = analogRead(LDR_PIN);

  int brightnessPercent;

  if (brightValueIsHigh == true) {
    brightnessPercent = map(ldrRaw, 0, 4095, 0, 100);
  } else {
    brightnessPercent = map(ldrRaw, 0, 4095, 100, 0);
  }

  brightnessPercent = constrain(brightnessPercent, 0, 100);

  bool isDark = brightnessPercent < darkThresholdPercent;

  if (isDark) {
    setLight(true);
    Serial.println("Status: DARK → Light ON");
  } else {
    setLight(false);
    Serial.println("Status: BRIGHT → Light OFF");
  }

  Serial.print("Brightness: ");
  Serial.print(brightnessPercent);
  Serial.print("%");
  Serial.print(" | Dark Threshold: ");
  Serial.print(darkThresholdPercent);
  Serial.println("%");

  Serial.println("-------------------------");

  delay(1000);
}

void setLight(bool state) {
  if (activeLowRelay == true) {
    digitalWrite(LIGHT_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
  }
}