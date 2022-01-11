#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUtil.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <properties.h>
#include <WiFiUdp.h>

#define XSTR(x) #x
#define STR(x) XSTR(x)

Adafruit_SSD1306 display(128, 64, &Wire, -1);

WiFiUtil wiFiUtil;

const long utcOffsetInSeconds = 25200;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);

float fVoltageMatrix[22][2] = {
    {4.2, 100},
    {4.15, 95},
    {4.11, 90},
    {4.08, 85},
    {4.02, 80},
    {3.98, 75},
    {3.95, 70},
    {3.91, 65},
    {3.87, 60},
    {3.85, 55},
    {3.84, 50},
    {3.82, 45},
    {3.80, 40},
    {3.79, 35},
    {3.77, 30},
    {3.75, 25},
    {3.73, 20},
    {3.71, 15},
    {3.69, 10},
    {3.61, 5},
    {3.27, 0},
    {0, 0}};

// https://theiotprojects.com/battery-status-monitoring-system-using-esp8266-arduino-iot-cloud
int getBatteryPercentage(int fVoltage)
{
  int i = 0;
  int perc = 100;
  for (i = 20; i >= 0; i--)
  {
    if (fVoltageMatrix[i][0] >= fVoltage)
    {
      perc = fVoltageMatrix[i + 1][1];
      break;
    }
  }
  return perc;
}

void setup()
{
  Serial.begin(115200);

  //Init WiFi
  wiFiUtil.setup();
  
  timeClient.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

size_t progress = 0;
DynamicJsonDocument doc(2048);

void displayMain(const char *msg)
{
  display.clearDisplay();

  //Headline progress
  display.setCursor(0, 0);
  char stat[6];
  sprintf(stat, "%i/%i", (progress + 1), doc.size());
  display.write(stat);
  display.display();

  //Clock
  display.setCursor(98, 0);
  char time[6];
  sprintf(time, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  display.write(time);
  display.display();

  //Headline
  display.setCursor(0, 12);
  display.write(msg);
  display.display();
}

void loop()
{
  if (doc.size() == 0)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, PARSEURL);
      http.GET();
      DeserializationError error = deserializeJson(doc, http.getStream());
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      http.end();
      timeClient.update();
      WiFi.disconnect();
      WiFi.forceSleepBegin();
      delay(1);
    }
    else
    {
      Serial.println("WiFi Disconnected");
      WiFi.forceSleepWake();
      wiFiUtil.setup();
      delay(100);
    }
  }
  else
  {
    if (progress < doc.size())
    {
      displayMain(doc[progress]);
      progress++;
    }
    else
    {
      progress = 0;
      doc.clear();
    }
    delay(50000);
  }
}