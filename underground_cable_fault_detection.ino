#define SW_VERSION "ThingSpeak.com"

// Wi-Fi
#include <WiFiClientSecure.h>

WiFiClient client;

const char* MY_SSID = "YOUR_WIFI_NAME";
const char* MY_PWD = "YOUR_WIFI_PASSWORD";

const char* TS_SERVER = "api.thingspeak.com";
String TS_API_KEY = "YOUR_THINGSPEAK_API_KEY";

void connectWifi()
{
  Serial.print("Connecting to ");
  Serial.print(MY_SSID);

  WiFi.begin(MY_SSID, MY_PWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("");
}

// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

float lat = 28.5458;
float lon = 77.1703;

SoftwareSerial gpsSerial(10, 14);
TinyGPS gps;

// Pin definitions
int ir = 13;
int Buzzer = 12;

int led = 5;
int led1 = 4;
int led2 = 2;

const int trigPin = 14;
const int echoPin = 15;

long duration;
float distance;

void setup()
{
  Serial.begin(9600);

  pinMode(ir, INPUT);
  pinMode(Buzzer, OUTPUT);

  pinMode(led, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Raspberry Pi Pico W I2C pins
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("AERO CARE");

  lcd.setCursor(0, 1);
  lcd.print("PREDICTOR");

  // Connect to Wi-Fi
  connectWifi();

  Serial.println("The GPS Received Signal:");

  // Initialize GPS
  gpsSerial.begin(9600);

  delay(1000);
}

void loop()
{
  // Read GPS data
  while (gpsSerial.available())
  {
    if (gps.encode(gpsSerial.read()))
    {
      gps.f_get_position(&lat, &lon);

      Serial.print("Position: ");
      Serial.print("Latitude:");
      Serial.print(lat, 6);
      Serial.print(";");

      Serial.print("Longitude:");
      Serial.println(lon, 6);

      Serial.print(lat);
      Serial.print(" ");
    }
  }

  String latitude = String(lat, 6);
  String longitude = String(lon, 6);

  Serial.println(latitude + ";" + longitude);

  // Display latitude
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("latitude:");
  lcd.println(latitude);

  delay(1000);

  // Display longitude
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("longitude:");
  lcd.println(longitude);

  delay(1000);

  // Read IR sensor
  int i = digitalRead(ir);

  Serial.print("ir: ");
  Serial.println(i);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IR SENSOR:");
  lcd.println(i);

  delay(1000);

  // Ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  // Calculate distance
  int distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.println(distance);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance:");
  lcd.println(distance);

  delay(1000);

  // IR sensor condition
  if (i == 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("wire is break");

    delay(1000);

    digitalWrite(Buzzer, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led1, LOW);
  }
  else
  {
    digitalWrite(Buzzer, LOW);

    digitalWrite(led, HIGH);
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
  }

  // Ultrasonic distance condition
  if (distance <= 50)
  {
    digitalWrite(Buzzer, HIGH);
  }
  else
  {
    digitalWrite(Buzzer, LOW);
  }

  // Send data to ThingSpeak
  if (client.connect(TS_SERVER, 80))
  {
    String postStr = TS_API_KEY;

    postStr += "&field1=";
    postStr += String(i);

    postStr += "&field2=";
    postStr += String(distance);

    postStr += "&field3=";
    postStr += String(i);

    postStr += "&field4=";
    postStr += String(latitude);

    postStr += "&field5=";
    postStr += String(longitude);

    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");

    client.print("X-THINGSPEAKAPIKEY: " + TS_API_KEY + "\n");

    client.print("Content-Type: application/x-www-form-urlencoded\n");

    client.print("Content-Length: ");
    client.print(postStr.length());

    client.print("\n\n");
    client.print(postStr);

    delay(1000);
  }
}