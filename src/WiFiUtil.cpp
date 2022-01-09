#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "WiFiUtil.h"

#define XSTR(x) #x
#define STR(x) XSTR(x)

WiFiUtil::WiFiUtil()
{
}

void WiFiUtil::setup()
{
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(STR(WIFI_SSID));
    WiFi.mode(WIFI_STA);
    WiFi.begin(STR(WIFI_SSID), STR(WIFI_PASS));

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}