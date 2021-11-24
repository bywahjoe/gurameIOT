#include <WiFi.h>
#include "pinku.h"
#include <BlynkSimpleEsp32.h>

//LCD & Servo
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

//Suhu
#include <OneWire.h>
#include <DallasTemperature.h>

//Water Level
#define readWaterHigh digitalRead(waterHigh)
#define readWaterMedium digitalRead(waterMedium)
#define readWaterLow digitalRead(waterLow)

LiquidCrystal_I2C lcd(0x27, 20, 4);
Servo myservo;

OneWire oneWire(suhupin);
DallasTemperature sensors(&oneWire);

BlynkTimer sendSensor;

float suhu = 0;
int turbi = 0;
int ph = 0;

int level=0;
String levelAir;

//Timer LCD
unsigned now = 0, before = 0;

BLYNK_WRITE(V5) {
  int klik = param.asInt();
  Serial.print("PAKAN"); Serial.println(klik);

  if (klik)bukaPakan();
  else tutupPakan();
}
BLYNK_WRITE(V14) {
  int klik = param.asInt();
  Serial.print("POMPA_IN :"); Serial.println(klik);

  if (klik)pompaINON();
  else pompaINOFF();
}
BLYNK_WRITE(V16) {
  int klik = param.asInt();
  Serial.print("POMPA_OUT :"); Serial.println(klik);

  if (klik)pompaOUTON();
  else pompaOUTOFF();
}
void pushSensor() {
  Blynk.virtualWrite(25, level);
  Blynk.virtualWrite(32, ph);
  Blynk.virtualWrite(33, suhu);
  Blynk.virtualWrite(35, turbi);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sensors.begin ();
  lcd.init();
  lcd.backlight();
  myservo.attach(servopin);
  pinMode(pompaIN, OUTPUT);
  pinMode(pompaOUT, OUTPUT);
  digitalWrite(pompaIN, HIGH);
  digitalWrite(pompaOUT, HIGH);

  pinMode(waterHigh, INPUT_PULLUP);
  pinMode(waterMedium, INPUT_PULLUP);
  pinMode(waterLow, INPUT_PULLUP);

  Blynk.begin(myblynk, ssid, pass);
  sendSensor.setInterval(3000L, pushSensor);

  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("~Gurame~");
  lcd.setCursor(5, 2);
  lcd.print("IoT System");
  delay(2000);

  now = millis();
  before = now;
}

void loop() {
  Serial.println(readWaterMedium);
  delay(20);

  now=millis();
  suhu = getSuhu();
  turbi=getTurbidity();
  ph=getPH();
  levelAir=getWaterLevel();

  //Refresh LCD 2 Detik
  if(now-before>=2000L){
  tampilLCD();
  before=now;
  }

  Blynk.run();
  sendSensor.run();
}
void tampilLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu :");
  lcd.print(suhu);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Turbi:");
  lcd.print(turbi);

  lcd.setCursor(0, 2);
  lcd.print("PH   :");
  lcd.print(ph);

  lcd.setCursor(0, 3);
  lcd.print("Level:");
  lcd.print(levelAir);
}
void pompaINON() {
  digitalWrite(pompaIN, LOW);
}
void pompaINOFF() {
  digitalWrite(pompaIN, HIGH);
}
void pompaOUTON() {
  digitalWrite(pompaOUT, LOW);
}
void pompaOUTOFF() {
  digitalWrite(pompaOUT, HIGH);
}
void bukaPakan() {
  myservo.write(0);
}
void tutupPakan() {
  myservo.write(90);
}
float getSuhu() {
  float val;
  sensors.requestTemperatures();
  val = sensors.getTempCByIndex(0);

  //  Serial.print("Suhu : "); Serial.println(val);
  return val;
}
int getTurbidity() {
  float V = 0, kekeruhan, VRata2, VHasil;
  int result;

  for (int i = 0; i < 800; i++)
  {
    V += ((float)analogRead(turbipin) / 4096) * 5;
  }

  VRata2 = V / 800;
  VHasil = roundf(VRata2 * 10.0f) / 10.0f;

  if (VHasil < 2.5)
  {
    kekeruhan = 3000;
  }
  else
  {
    kekeruhan = -1120.4 * sq(VHasil) + 5742.3 * VHasil - 4353.8;
  }

  result = kekeruhan;
  Serial.print("tegangan :");
  Serial.print(VHasil);
  Serial.print(" V");

  Serial.print("\t kekeruhan :");
  Serial.println(kekeruhan);

  return result;
}
float getPH() {
  float voltage = 0;
  int nilaiAnalog = 0;
  float nilaiPH = 0;

  nilaiAnalog = analogRead(phpin);

  //rumus nilai tegangan
  voltage = nilaiAnalog * (5.0 / 4095);

  //rumus nilai ph air
  nilaiPH = 2.676 * voltage - 0.3130;;

  //  Serial.print("Nilai Analog : ");
  //  Serial.print(nilaiAnalog);
  //  Serial.print("      Nilai Voltage : ");
  //  Serial.print(voltage);
  //  Serial.print("          Nilai PH : ");
  //  Serial.print(nilaiPH);

  return nilaiPH;
}
String getWaterLevel() {
  String result;
  
  if (!readWaterHigh) {
    result = "PENUH";
    level=10;
  }
  else if (!readWaterMedium) {
    result = "MEDIUM";
    level=5;
  }
  else {
    result = "KURANG";
    level=0;
  }
  return result;
}
