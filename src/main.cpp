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

WiFiUtil wiFiUtil;

const uint8_t PIN_ROTARY_A = 12;
const uint8_t PIN_ROTARY_B = 14;

//Clock
const long utcOffsetInSeconds = 25200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);

//Display
Adafruit_SSD1306 display(128, 64, &Wire, -1);
int displayDelay = 0;

size_t progress = 0;
DynamicJsonDocument doc(2048);

void setup()
{
  Serial.begin(115200);

  wiFiUtil.setup();

  timeClient.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);

  pinMode(PIN_ROTARY_A, INPUT_PULLUP);
  pinMode(PIN_ROTARY_B, INPUT);
}

void displayMain(const char *msg)
{
  display.clearDisplay();

  //Progress
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

  //Main Text
  display.setCursor(0, 12);
  display.write(msg);
  display.display();
}

int prevA = 0;
int prevB = 0;

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
    if (displayDelay > 50000)
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
      displayDelay = 0;
    }
    displayDelay++;
    delay(1);
  }

  byte buttonStateA = digitalRead(PIN_ROTARY_A);
  byte buttonStateB = digitalRead(PIN_ROTARY_B);

  if (prevA != buttonStateA && prevB == buttonStateB)
  {
    if (buttonStateB == buttonStateA)
    {
      if (progress < doc.size() - 1)
      {
        progress++;
      }
      else
      {
        progress = 0;
      }
    }
    else
    {
      if (progress > 0)
      {
        progress--;
      }
      else
      {
        progress = doc.size() - 1;
      }
    }

    displayMain(doc[progress]);
    displayDelay = 0;
  }

  prevA = buttonStateA;
  prevB = buttonStateB;
}
