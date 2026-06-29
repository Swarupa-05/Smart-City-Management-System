#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define SDA_PIN 32
#define SCL_PIN 33

// ---------- DHT22 ----------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------- LDR ----------
#define LDR_PIN 34

// ---------- Traffic LEDs ----------
#define RED_LED 18
#define YELLOW_LED 19
#define GREEN_LED 21

// ---------- Street Light ----------
#define STREET_LED 15

// ---------- Traffic Button ----------
#define TRAFFIC_BUTTON 25

// ---------- Water Tank Ultrasonic ----------
#define WATER_TRIG 12
#define WATER_ECHO 13

// ---------- Dustbin Ultrasonic ----------
#define BIN_TRIG 14
#define BIN_ECHO 27

// ---------- Air Quality (Potentiometer) ----------
#define AIR_PIN 35

// ---------- Buzzer ----------
#define BUZZER 23

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long distance = duration * 0.034 / 2;

  if (distance == 0) return 999;
  return distance;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (1);
  }

  dht.begin();

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(STREET_LED, OUTPUT);

  pinMode(TRAFFIC_BUTTON, INPUT_PULLUP);

  pinMode(WATER_TRIG, OUTPUT);
  pinMode(WATER_ECHO, INPUT);

  pinMode(BIN_TRIG, OUTPUT);
  pinMode(BIN_ECHO, INPUT);

  pinMode(BUZZER, OUTPUT);

  // Initial Traffic State
  digitalWrite(GREEN_LED, HIGH);
}

void loop() {

  // ---------- Read Sensors ----------
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  int lightLevel = analogRead(LDR_PIN);
  int airQuality = analogRead(AIR_PIN);

  long waterDistance = getDistance(WATER_TRIG, WATER_ECHO);
  long binDistance = getDistance(BIN_TRIG, BIN_ECHO);

  bool trafficRequest = !digitalRead(TRAFFIC_BUTTON);

  // ---------- Smart Street Light ----------
  if (lightLevel < 2000) {
    digitalWrite(STREET_LED, HIGH);
  } else {
    digitalWrite(STREET_LED, LOW);
  }

  // ---------- Traffic Signal ----------
  // Normal State
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // Vehicle Detected
  if (trafficRequest) {

    // Green -> Red
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    delay(5000);

    // Red -> Yellow
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    delay(2000);

    // Back to Green
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }

  // ---------- Emergency Alerts ----------
  if (airQuality > 3500 ||
      binDistance < 10 ||
      waterDistance < 10) {

    tone(BUZZER, 1000);

  } else {

    noTone(BUZZER);
  }

  // ---------- OLED Display ----------
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("T:");
  display.print(temp, 1);
  display.print("C");

  display.setCursor(64, 0);
  display.print("H:");
  display.print(hum, 0);
  display.print("%");

  display.setCursor(0, 12);
  display.print("Light:");
  display.print(lightLevel);

  display.setCursor(0, 24);

  if (airQuality < 1000)
    display.print("Air:GOOD");
  else if (airQuality < 2500)
    display.print("Air:MOD");
  else if (airQuality < 3500)
    display.print("Air:POOR");
  else
    display.print("Air:BAD");

  display.setCursor(0, 36);

  if (waterDistance < 10)
    display.print("Water:FULL");
  else if (waterDistance < 30)
    display.print("Water:MED");
  else
    display.print("Water:LOW");

  display.setCursor(0, 48);

  if (binDistance < 10)
    display.print("Bin:FULL");
  else if (binDistance < 25)
    display.print("Bin:MED");
  else
    display.print("Bin:EMPTY");

  display.display();

  // ---------- Serial Monitor ----------
  Serial.println("===== SMART CITY MANAGEMENT =====");
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  Serial.print("Light Level: ");
  Serial.println(lightLevel);

  Serial.print("Air Quality: ");
  Serial.println(airQuality);

  Serial.print("Water Distance: ");
  Serial.print(waterDistance);
  Serial.println(" cm");

  Serial.print("Dustbin Distance: ");
  Serial.print(binDistance);
  Serial.println(" cm");

  Serial.println("--------------------------------");

  delay(500);
}