// ============================================================
//   SMART INFANT INCUBATOR — Ansh Dubey
//   Features: Dual Sensor + PID Control + Gas Alert + LCD
//   ESP32 | DHT22 | DS18B20 | MQ2 | 4 Relays | Buzzer
// ============================================================
#define BLYNK_TEMPLATE_ID "TMPL3m0G3VblT"
#define BLYNK_TEMPLATE_NAME "Incubator"
#define BLYNK_AUTH_TOKEN "vXDM5nL7PXWJKPQV5o5rozxPd89JqR9L"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// ──────────────── PIN DEFINITIONS ────────────────

char ssid[] = "Wokwi-GUEST";
char pass[] = "";
#define DHTPIN          4
#define DHTTYPE         DHT22
#define DS18B20_PIN     16
#define MQ2_PIN         34
#define BUZZER_PIN      23
#define RELAY_HEATER    19
#define RELAY_FAN       18
#define RELAY_HUMIDIFIER 17
#define RELAY_PUMP      13

// ──────────────── INCUBATOR SETPOINTS ────────────────
#define TEMP_SETPOINT   37.0
#define HUMID_LOW       50.0
#define HUMID_HIGH      70.0
#define TEMP_DANGER     39.5
#define SENSOR_FAULT_THRESHOLD 2.0
#define GAS_THRESHOLD   2000

// ──────────────── PID PARAMETERS ────────────────
#define PID_KP   8.0
#define PID_KI   0.5
#define PID_KD   2.0
#define PID_MIN  0
#define PID_MAX  100

// ──────────────── OBJECTS ────────────────
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ──────────────── PID VARIABLES ────────────────
float pidOutput    = 0;
float prevError    = 0;
float integral     = 0;
unsigned long lastPIDTime = 0;

// ──────────────── SYSTEM STATE ────────────────
float dhtTemp      = 0;
float dsTemp       = 0;
float avgTemp      = 0;
float humidity     = 0;
int   gasValue     = 0;
bool  heaterOn     = false;
bool  fanOn        = false;
bool  humidOn      = false;
bool  pumpOn       = false;
bool  alarmOn      = false;
bool  sensorFault  = false;
bool  gasAlert     = false;

unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate  = 0;
unsigned long lastSerialLog  = 0;

// ──────────────── CUSTOM LCD CHARS ────────────────
byte degreeChar[8] = {0x07,0x05,0x07,0x00,0x00,0x00,0x00,0x00};
byte heartChar[8]  = {0x00,0x0A,0x1F,0x1F,0x0E,0x04,0x00,0x00};
byte bellChar[8]   = {0x04,0x0E,0x0E,0x0E,0x1F,0x00,0x04,0x00};
byte checkChar[8]  = {0x00,0x01,0x03,0x16,0x1C,0x08,0x00,0x00};

// ============================================================
float computePID(float setpoint, float measured) {
  unsigned long now = millis();
  float dt = (now - lastPIDTime) / 1000.0;
  if (dt <= 0) dt = 0.1;
  lastPIDTime = now;

  float error = setpoint - measured;
  integral += error * dt;
   
   if(abs(error) < 0.2)
{
    integral = 0;
}
  integral = constrain(integral, -50, 50);
  float derivative = (error - prevError) / dt;
  prevError = error;

  float output = (PID_KP * error) + (PID_KI * integral) + (PID_KD * derivative);
  return constrain(output, PID_MIN, PID_MAX);
}

void setRelay(int pin, bool state) {
  digitalWrite(pin, state ? LOW : HIGH);
}

void readSensors() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) { humidity = h; dhtTemp = t; }

  ds18b20.requestTemperatures();
  float ds = ds18b20.getTempCByIndex(0);
  if (ds != DEVICE_DISCONNECTED_C) dsTemp = ds;

  
  sensorFault = (abs(dhtTemp - dsTemp) > SENSOR_FAULT_THRESHOLD);

if(sensorFault)
{
    avgTemp = dsTemp;
}
else
{
    avgTemp = (dhtTemp + dsTemp) / 2.0;
}

gasValue = (
    analogRead(MQ2_PIN) +
    analogRead(MQ2_PIN) +
    analogRead(MQ2_PIN)
) / 3;
gasAlert = (gasValue > GAS_THRESHOLD);
}
  
void runControl() {
  
  if(!sensorFault)
{
    pidOutput = computePID(TEMP_SETPOINT, avgTemp);
}
else
{
    pidOutput = 0;
}
  static bool heaterState = false;

if(avgTemp < (TEMP_SETPOINT - 0.5))
{
    heaterState = true;
}
else if(avgTemp > (TEMP_SETPOINT + 0.5))
{
    heaterState = false;
}

heaterOn = heaterState;
if(pidOutput < 10)
{
    heaterOn = false;
}
if(sensorFault)
{
    heaterOn = false;
    fanOn = true;
}

if(sensorFault || gasAlert || avgTemp > (TEMP_SETPOINT + 1.0)) {
    fanOn = true;
} else {
    fanOn = false;
}
  if(humidity < 1 || humidity > 100)
{
    humidOn = false;
    pumpOn  = false;
}
else
{
    if (humidity < HUMID_LOW)
{
    humidOn = true;
    pumpOn = true;
}
else if (humidity >= HUMID_HIGH)
{
    humidOn = false;
    pumpOn = false;
}
}
  setRelay(RELAY_HEATER,     heaterOn);
  setRelay(RELAY_FAN,        fanOn);
  setRelay(RELAY_HUMIDIFIER, humidOn);
  setRelay(RELAY_PUMP,       pumpOn);

  alarmOn = (avgTemp > TEMP_DANGER || gasAlert || sensorFault);
  digitalWrite(BUZZER_PIN, alarmOn ? HIGH : LOW);
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.write(byte(1));
  lcd.print(" INFANT INCUBATOR  ");
  lcd.write(byte(1));

  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(avgTemp, 1);
  lcd.write(byte(0));
  lcd.print("C PID:");
  lcd.print((int)pidOutput);
  lcd.print("%   ");

  lcd.setCursor(0, 2);
  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("% GAS:");
  lcd.print(gasValue);
  lcd.print("    ");

  lcd.setCursor(0, 3);
  if (sensorFault)            { lcd.print("!! SENSOR FAULT !!  "); }
  else if (gasAlert)          { lcd.print("!! GAS ALERT !!     "); }
  else if (avgTemp > TEMP_DANGER) { lcd.print("!! OVERHEAT !!      "); }
  else {
    lcd.print("SYS OK ");

lcd.print("H:");
lcd.print(heaterOn ? "1 " : "0 ");

lcd.print("F:");
lcd.print(fanOn ? "1 " : "0 ");

lcd.print("HM:");
lcd.print(humidOn ? "1" : "0");
  }
}

void serialLog() {
  Serial.println("========================================");
  Serial.print("DHT22  : "); Serial.print(dhtTemp, 2); Serial.println(" C");
  Serial.print("DS18B20: "); Serial.print(dsTemp, 2);  Serial.println(" C");
  Serial.print("Avg    : "); Serial.print(avgTemp, 2);  Serial.println(" C");
  Serial.print("Humidity: "); Serial.print(humidity, 1); Serial.println(" %");
  Serial.print("Gas ADC: "); Serial.println(gasValue);
  Serial.print("PID Out: "); Serial.print(pidOutput, 1); Serial.println(" %");
  Serial.print("Heater : "); Serial.println(heaterOn ? "ON" : "OFF");
  Serial.print("Fan    : "); Serial.println(fanOn    ? "ON" : "OFF");
  Serial.print("Humid  : "); Serial.println(humidOn  ? "ON" : "OFF");
  Serial.print("Fault  : "); Serial.println(sensorFault ? "YES" : "NO");
  Serial.print("Alarm  : "); Serial.println(alarmOn  ? "YES" : "NO");
  Serial.println("========================================");
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();
  ds18b20.begin();

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, degreeChar);
  lcd.createChar(1, heartChar);
  lcd.createChar(2, bellChar);
  lcd.createChar(3, checkChar);

  pinMode(RELAY_HEATER,     OUTPUT); digitalWrite(RELAY_HEATER,     HIGH);
  pinMode(RELAY_FAN,        OUTPUT); digitalWrite(RELAY_FAN,        HIGH);
  pinMode(RELAY_HUMIDIFIER, OUTPUT); digitalWrite(RELAY_HUMIDIFIER, HIGH);
  pinMode(RELAY_PUMP,       OUTPUT); digitalWrite(RELAY_PUMP,       HIGH);
  pinMode(BUZZER_PIN,       OUTPUT); digitalWrite(BUZZER_PIN,       LOW);

  lcd.setCursor(3, 1); lcd.print("SMART INCUBATOR");
  lcd.setCursor(4, 2); lcd.print("Ansh Dubey");
  delay(2000);
  lcd.clear();

  lastPIDTime = millis();
  Serial.println("System Ready!");
}

void loop() {
 
  unsigned long now = millis();

  if (now - lastSensorRead >= 2000) {
    readSensors();
    runControl();
    Blynk.virtualWrite(V0, avgTemp);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, gasValue);
    Blynk.virtualWrite(V3, pidOutput);
    Blynk.virtualWrite(V4, heaterOn);
    Blynk.virtualWrite(V5, fanOn);
    Blynk.virtualWrite(V6, alarmOn);
    Blynk.virtualWrite(V7, humidOn);
    Blynk.virtualWrite(V8, pumpOn);
    lastSensorRead = now;
  }
  if (now - lastLCDUpdate >= 500) {
    updateLCD();
    lastLCDUpdate = now;
  }
  if (now - lastSerialLog >= 5000) {
    serialLog();
    lastSerialLog = now;
  }
   Blynk.run();
}
