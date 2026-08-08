#include <WiFi.h>
#include <WebServer.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <OneWire.h>
#include <DallasTemperature.h>


// -------- WIFI --------

const char* ssid = "SmartRiverGuardian";
const char* password = "12345678";

WebServer server(80);


// -------- TFT --------

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);


// -------- SENSOR PINS --------

#define TRIG_PIN 12
#define ECHO_PIN 14

#define TEMP_PIN 27

#define TURBIDITY_PIN 34

#define BUZZER 26



// -------- DS18B20 --------

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);



// -------- VARIABLES --------

float temperature = 0;
int turbidity = 0;

float distance = 0;
float level = 0;

String status = "WATER CLEAN";



// -------- ULTRASONIC --------

float readDistance()
{

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);


  long duration = pulseIn(ECHO_PIN, HIGH, 30000);


  if(duration == 0)
  {
    return 999;
  }


  return duration * 0.0343 / 2;

}



// -------- SEND DATA TO WEBSITE --------

void sendData()
{

String json = "{";


json += "\"temperature\":";
json += String(temperature,1);


json += ",\"turbidity\":";
json += String(turbidity);


json += ",\"distance\":";
json += String(distance,1);


json += ",\"level\":";
json += String(level,1);


json += ",\"status\":\"";
json += status;
json += "\"";


json += "}";


server.send(200,"application/json",json);

}



// -------- SETUP --------

void setup()
{

Serial.begin(115200);


// TFT

tft.begin();

tft.setRotation(1);

tft.fillScreen(ILI9341_BLACK);

tft.setTextSize(2);

tft.setTextColor(ILI9341_GREEN);


tft.setCursor(20,30);

tft.println("SMART RIVER");


tft.setCursor(20,60);

tft.println("GUARDIAN AI");


delay(2000);



// Sensors

pinMode(TRIG_PIN, OUTPUT);

pinMode(ECHO_PIN, INPUT);


pinMode(BUZZER, OUTPUT);



tempSensor.begin();



// ESP32 WIFI HOTSPOT

WiFi.softAP(ssid,password);


Serial.println("WiFi Started");

Serial.print("IP Address: ");

Serial.println(WiFi.softAPIP());



// Web Server

server.on("/data", sendData);

server.begin();



tft.fillScreen(ILI9341_BLACK);

tft.setCursor(10,20);

tft.println("WiFi Ready");


tft.setCursor(10,60);

tft.println("192.168.4.1");


delay(3000);


}



// -------- LOOP --------

void loop()
{

server.handleClient();


// Temperature

tempSensor.requestTemperatures();

temperature = tempSensor.getTempCByIndex(0);


// Turbidity

turbidity = analogRead(TURBIDITY_PIN);


// Distance

distance = readDistance();



// Water Level

if(distance <= 50 && distance >= 5)
{

level = ((50 - distance) * 100) / 45;

}

else if(distance < 5)
{

level = 100;

}

else
{

level = 0;

}



// Water Pollution Detection

if(turbidity > 2500)
{
  digitalWrite(BUZZER, HIGH);
  status = "POLLUTED WATER";
}
else
{
  digitalWrite(BUZZER, LOW);
  status = "WATER CLEAN";
}



// -------- TFT DISPLAY --------

tft.fillScreen(ILI9341_BLACK);

tft.setTextSize(2);



tft.setTextColor(ILI9341_CYAN);

tft.setCursor(10,10);

tft.println("LIVE MONITOR");



tft.setCursor(10,50);

tft.print("Temp: ");

tft.print(temperature);

tft.println(" C");



tft.setCursor(10,90);

tft.print("Turb: ");

tft.println(turbidity);



tft.setCursor(10,130);

tft.print("Dist: ");

tft.print(distance);

tft.println(" cm");



tft.setCursor(10,170);

tft.print("Level: ");

tft.print(level);

tft.println("%");



tft.setCursor(10,210);


if(status == "POLLUTED WATER")
{
  tft.setTextColor(ILI9341_RED);
  tft.println("POLLUTED WATER");
}
else
{
  tft.setTextColor(ILI9341_GREEN);
  tft.println("WATER CLEAN");
}



// Serial Monitor

Serial.print("Temperature: ");
Serial.println(temperature);

Serial.print("Turbidity: ");
Serial.println(turbidity);

Serial.print("Distance: ");
Serial.println(distance);

Serial.print("Level: ");
Serial.println(level);

Serial.print("Status: ");
Serial.println(status);

Serial.println("----------------");


delay(1000);

}