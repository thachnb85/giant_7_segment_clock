#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <WebServer.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <HTTPClient.h>

#include "FS.h"
#include "WiFi.h"
#include "SPIFFS.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))
//=====================================================================================================
// Web server
//ESP8266WebServer server(80);
//ESP8266HTTPUpdateServer httpUpdateServer;
//===================================================================================================
// RGBW led setup, connect D13 to Led control.
#define LED_PIN 13
#define serialRate 9600
#define NUM_LEDS 58 // Total 58 leds for 2 leds/segment design.

// D12 to EN pin on ESP32

// Setup the LED strips.
Adafruit_NeoPixel LEDs(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);
//====================================================================================================
// Wifi Setup
#define WIFIMODE 2 // 0 = Only Soft Access Point, 1 = Only connect to local WiFi network with UN/PW, 2 = Both

#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
const char *APssid = "CLOCK_AP";
const char *APpassword = "1234567890";
#endif

#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
#include "credential.h" // Create this file in the same directory as the .ino file and add your credentials (#define SID YOURSSID and on the second line #define PW YOURPASSWORD)
#endif
//=====================================================================================================
// NTP SERVER

// For UTC -6.00 : -6 * 60 * 60 : -21600 (Saskatoon)
// For UTC +1.00 : 1 * 60 * 60 : 3600
// For UTC +0.00 : 0 * 60 * 60 : 0

const long utcOffsetInSeconds = -21600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//=====================================================================================================
// 649
long lastNumber = 0;

// Settings

float temperatureNow = 0;

// We update the time every second
int previousSeconds = 0;

unsigned long prevTime = 0;
// day color
byte r_val = 0;
byte g_val = 255;
byte b_val = 0;

// night color
byte r_val_night = 80;
byte g_val_night = 0;
byte b_val_night = 0;

// dot color index
int dotColorIndex = 0;
uint32_t dotColors[8] = {
    LEDs.Color(255, 0, 0),   //red
    LEDs.Color(255, 128, 0), //orange
    LEDs.Color(255, 255, 0), //yellow
    LEDs.Color(0, 255, 0),   // green
    LEDs.Color(0, 255, 255), //cyan
    LEDs.Color(0, 128, 255), //light blue
    LEDs.Color(0, 0, 255),   // dark blue
    LEDs.Color(255, 0, 255), // purple
};

bool isNightColor = false;

bool dotsOn = true;
byte brightness = 255;
float temperatureCorrection = -3.0;
byte temperatureSymbol = 12; // 12=Celcius, 13=Fahrenheit check 'numbers'
byte clockMode = 0;          // Clock modes: 0=Clock, 1=Temperature
unsigned long countdownMilliSeconds;
unsigned long endCountDownMillis;
byte hourFormat = 12; // Change this to 12 if you want default 12 hours format instead of 24
uint32_t countdownColor = LEDs.Color(0, 255, 0);
byte scoreboardLeft = 0;
byte scoreboardRight = 0;
uint32_t scoreboardColorLeft = LEDs.Color(255, 0, 0);
uint32_t scoreboardColorRight = LEDs.Color(0, 255, 0);
uint32_t alternateColor = LEDs.Color(0, 0, 0);

/*
   * 
      __ __        __ __          __ __         _8 _9   
    __     __    __     __      __     __     7       10
    __     __    __     __  28  __     __    _6       11
      __ __       __  __         __  __        _13  _12  
    __     44    __     30  29  __     14    _5        0
    __     __    __     __      __     __    _4        1
      __ __       __ __           __ __       _  _3 _2   

   */
// This is the number with 2 leds in each segment
long numbers[] = {
    0b00111111111111, // [0] 0
    0b00110000000011, // [1] 1
    0b11111100111100, // [2] 2
    0b11111100001111, // [3] 3
    0b11110011000011, // [4] 4
    0b11001111001111, // [5] 5
    0b11001111111111, // [6] 6
    0b00111100000011, // [7] 7
    0b11111111111111, // [8] 8
    0b11111111001111, // [9] 9
    0b00000000000000, // [10] off
    0b11111111000000, // [11] degrees symbol
    0b00001111111100, // [12] C(elsius)
    0b11001111110000, // [13] F(ahrenheit)
    0b11000000000000, // [14] MINUS sign
};

int numOfLedPerSegment = 14;
///==================================================================================
/// Function prototype
void displayDots(uint32_t color);
void updateClock(int hour, int mins, int secs);
void updateTemperature();
void allBlank();
String httpGETRequest(const char *serverName);
void queryTemperature();
void displayColorfulDots();

String httpGETRequest(const char *serverName)
{
  yield();
  HTTPClient http;
  http.begin(serverName);

  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0)
  {
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}

///==================================================================================
void setup()
{
  Serial.begin(115200);

  pinMode(COUNTDOWN_OUTPUT, OUTPUT);

  delay(200);

  // LED Startup Sequence
  LEDs.begin();

  // Comment out for silent restarting
  // flash red then green then blue for testing
  // for (int i = 0; i < NUM_LEDS; i++)
  // {
  //   LEDs.setPixelColor(i, LEDs.Color(80, 0, 0));
  // }
  // LEDs.show();
  // delay(500);

  // for (int i = 0; i < NUM_LEDS; i++)
  // {
  //   LEDs.setPixelColor(i, LEDs.Color(0, 80, 0));
  // }
  // LEDs.show();
  // delay(500);

  // for (int i = 0; i < NUM_LEDS; i++)
  // {
  //   LEDs.setPixelColor(i, LEDs.Color(0, 0, 80));
  // }
  // LEDs.show();
  // delay(500);

  // // clear all leds
  // allBlank();
  // LEDs.clear();
  // LEDs.show();

//  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  delay(200);

  // WiFi - AP Mode or both
//#if defined(WIFIMODE) && (WIFIMODE == 0 || WIFIMODE == 2)
//  WiFi.mode(WIFI_AP_STA);
//  WiFi.softAP(APssid, APpassword); // IP is usually 192.168.4.1
//  Serial.println();
//  Serial.print("SoftAP IP: ");
//  Serial.println(WiFi.softAPIP());
//#endif

  // WiFi - Local network Mode or both
#if defined(WIFIMODE) && (WIFIMODE == 1 || WIFIMODE == 2)
  byte count = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    // Stop if cannot connect
    if (count >= 50)
    {
      Serial.println("Could not connect to local WiFi. Self-reset");
      ESP.restart();
    }

    delay(500);
    Serial.print(".");

    //// Comment out for silent restarting
    // // Showing one by one led till wifi is connected.
    // for (int i = 0; i < count; i++)
    // {
    //   LEDs.setPixelColor(i, LEDs.Color(0, 255, 0));
    // }
    // LEDs.show();
    count++;
  }

  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  IPAddress ip = WiFi.localIP();
  Serial.println(ip[3]);
#endif

  SPIFFS.begin();

  timeClient.begin();
  digitalWrite(COUNTDOWN_OUTPUT, LOW);

  queryTemperature();
}


void loop()
{
  timeClient.update();

  int currentHour = timeClient.getHours();
  int currentMin = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  isNightColor = (currentHour >= 23) || (currentHour < 7) || (currentHour == 7 && currentMin < 30);

  if ((currentSecond == 10) && (currentMin % 10 == 0 )){
    Serial.print("Reset the controller.............");
    ESP.restart();
  }

  if (currentSecond - previousSeconds != 0)
  {
    previousSeconds = currentSecond;
    if (currentSecond % 600 == 0) // every 10 minutes
    {
      queryTemperature();
    }

    if (clockMode == 0)
    {
      if ((currentSecond >= 3 && currentSecond <= 6) || (currentSecond >= 33 && currentSecond <= 36)) 
      {
        updateTemperature();
      }
      else
      {
        updateClock(currentHour, currentMin, currentSecond);
      }
    }
    else if (clockMode == 1)
    {
      updateTemperature();
    }

    LEDs.setBrightness(brightness);
    LEDs.show();
  }
}

void displayNumber(byte number, byte segment, uint32_t color)
{
  /*
   * 
      __ __        __ __          __ __         _8 _9   
    __     __    __     __      __     __     7       10
    __     __    __     __  28  __     __    _6       11
      __ __       __  __         __  __        _13  _12  
    __     44    __     30  29  __     14    _5        0
    __     __    __     __      __     __    _4        1
      __ __       __ __           __ __       _  _3 _2   

   */

  // segment from left to right: 3, 2, 1, 0
  byte startindex = 0;
  switch (segment)
  {
  case 0:
    startindex = 0;
    break;
  case 1:
    startindex = 14;
    break;
  case 2:
    startindex = 30;
    break;
  case 3:
    startindex = 44;
    break;
  }

  // set color for 1 number, start from index
  for (byte i = 0; i < numOfLedPerSegment; i++)
  {
    yield();
    LEDs.setPixelColor(i + startindex, ((numbers[number] & 1 << i) == 1 << i) ? color : alternateColor);
  }
}

void allBlank()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    yield();
    LEDs.setPixelColor(i, LEDs.Color(0, 0, 0));
  }
  LEDs.show();
}

void updateClock(int hour, int mins, int secs)
{
  uint32_t color = LEDs.Color(r_val, g_val, b_val);
  if (isNightColor)
  {
    color = LEDs.Color(r_val_night, g_val_night, b_val_night);
  }else{
    dotColorIndex++;
    if (dotColorIndex >7) dotColorIndex = 0;

    // color change every minute
    color = dotColors[mins % 8];
  }

  if (hourFormat == 12 && hour > 12)
    hour = hour - 12;

  byte h1 = hour / 10;
  byte h2 = hour % 10;
  byte m1 = mins / 10;
  byte m2 = mins % 10;
  // byte s1 = secs / 10;
  // byte s2 = secs % 10;

  if (h1 > 0)
    displayNumber(h1, 3, color);
  else
    displayNumber(10, 3, color); // Blank

  displayNumber(h2, 2, color);
  displayNumber(m1, 1, color);
  displayNumber(m2, 0, color);

  if (isNightColor){
    displayDots(color);
  }else{
    displayColorfulDots();
  }
}

void updateCountdown()
{

  if (countdownMilliSeconds == 0 && endCountDownMillis == 0)
    return;

  unsigned long restMillis = endCountDownMillis - millis();
  unsigned long hours = ((restMillis / 1000) / 60) / 60;
  unsigned long minutes = (restMillis / 1000) / 60;
  unsigned long seconds = restMillis / 1000;
  int remSeconds = seconds - (minutes * 60);
  int remMinutes = minutes - (hours * 60);

  Serial.print(restMillis);
  Serial.print(" ");
  Serial.print(hours);
  Serial.print(" ");
  Serial.print(minutes);
  Serial.print(" ");
  Serial.print(seconds);
  Serial.print(" | ");
  Serial.print(remMinutes);
  Serial.print(" ");
  Serial.println(remSeconds);

  byte h1 = hours / 10;
  byte h2 = hours % 10;
  byte m1 = remMinutes / 10;
  byte m2 = remMinutes % 10;
  byte s1 = remSeconds / 10;
  byte s2 = remSeconds % 10;

  uint32_t color = countdownColor;
  if (restMillis <= 60000)
  {
    color = LEDs.Color(255, 0, 0);
  }

  if (hours > 0)
  {
    // hh:mm
    displayNumber(h1, 3, color);
    displayNumber(h2, 2, color);
    displayNumber(m1, 1, color);
    displayNumber(m2, 0, color);
  }
  else
  {
    // mm:ss
    displayNumber(m1, 3, color);
    displayNumber(m2, 2, color);
    displayNumber(s1, 1, color);
    displayNumber(s2, 0, color);
  }

  displayDots(color);

  if (hours <= 0 && remMinutes <= 0 && remSeconds <= 0)
  {
    Serial.println("Countdown timer ended.");
    //endCountdown();
    countdownMilliSeconds = 0;
    endCountDownMillis = 0;
    digitalWrite(COUNTDOWN_OUTPUT, HIGH);
    return;
  }
}

void endCountdown()
{
  allBlank();
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if (i > 0)
      LEDs.setPixelColor(i - 1, LEDs.Color(0, 0, 0));

    LEDs.setPixelColor(i, LEDs.Color(255, 0, 0));
    LEDs.show();
    delay(25);
  }
}

// Dot is 28, 29 for 2 led/segment design
void displayDots(uint32_t color)
{
  if (dotsOn)
  {
    LEDs.setPixelColor(28, color);
    LEDs.setPixelColor(29, color);
  }
  else
  {
    LEDs.setPixelColor(28, LEDs.Color(0, 0, 0));
    LEDs.setPixelColor(29, LEDs.Color(0, 0, 0));
  }

  dotsOn = !dotsOn;
}

void displayColorfulDots()
{
  uint32_t color = dotColors[dotColorIndex];

  if (dotsOn)
  {
    LEDs.setPixelColor(28, color);
    LEDs.setPixelColor(29, color);
  }
  else
  {
    LEDs.setPixelColor(28, LEDs.Color(0, 0, 0));
    LEDs.setPixelColor(29, LEDs.Color(0, 0, 0));
  }

  dotsOn = !dotsOn;
}

void hideDots()
{
  LEDs.setPixelColor(28, LEDs.Color(0, 0, 0));
  LEDs.setPixelColor(29, LEDs.Color(0, 0, 0));
}

void updateTemperature()
{
  float ctemp = temperatureNow;

  // Convert to F
  if (temperatureSymbol == 13)
    ctemp = (ctemp * 1.8000) + 32;

  bool printMinusSign = false;
  if (ctemp < 0)
  {
    printMinusSign = true;
    ctemp = abs(ctemp);
  }
  byte t1 = int(abs(ctemp)) / 10;
  byte t2 = int(ctemp) % 10;

  uint32_t color = LEDs.Color(r_val, g_val, b_val);
  if (isNightColor)
  {
    color = LEDs.Color(r_val_night, g_val_night, b_val_night);
  }

  // we dont' need to show zero
  if (t1 == 0)
  {
    if (printMinusSign)
    {
      displayNumber(14, 3, color);
    }
    displayNumber(t2, 2, color);
    displayNumber(11, 1, color);                // degree symbol
    displayNumber(temperatureSymbol, 0, color); // temp Unit.
  }
  else
  {
    if (printMinusSign)
    {
      displayNumber(14, 3, color);
    }
    displayNumber(t1, 2, color);
    displayNumber(t2, 1, color);
    displayNumber(11, 0, color); // degree symbol
  }

  hideDots();
}

void updateScoreboard()
{
  byte sl1 = scoreboardLeft / 10;
  byte sl2 = scoreboardLeft % 10;
  byte sr1 = scoreboardRight / 10;
  byte sr2 = scoreboardRight % 10;

  displayNumber(sl1, 3, scoreboardColorLeft);
  displayNumber(sl2, 2, scoreboardColorLeft);
  displayNumber(sr1, 1, scoreboardColorRight);
  displayNumber(sr2, 0, scoreboardColorRight);
  hideDots();
}

void queryTemperature()
{
  yield();
  // data returning from api.openweathermap.org
  // temp unit is Kevin
  String payload = httpGETRequest(weatherURL);
  JSONVar myObject = JSON.parse(payload);
  if (JSON.typeof(myObject) == "undefined")
  {
    Serial.println("Parsing input failed!");
    return;
  }
  JSONVar value = myObject["main"]["feels_like"];
  temperatureNow = double(value) - 273.15; // in *C
}

void displayTemperature()
{
  byte t1 = int(temperatureNow) / 10;
  byte t2 = int(temperatureNow) % 10;

  uint32_t color = LEDs.Color(r_val, g_val, b_val);

  displayNumber(t1, 3, color);
  displayNumber(t2, 2, color);
  displayNumber(11, 1, color);
  displayNumber(temperatureSymbol, 0, color);
  hideDots();
}
