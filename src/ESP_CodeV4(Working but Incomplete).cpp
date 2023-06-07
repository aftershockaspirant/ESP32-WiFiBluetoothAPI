#include <Arduino.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define btcServerName "ESP32_MarvelMovies"

BluetoothSerial SerialBT;

String data = "";
String teamId = "";

void handleAction(String action, DynamicJsonDocument doc)
{
    if (action == "getNetworks")
    {
        teamId = doc["teamId"].as<const char *>();
        String ssid = doc["ssid"].as<const char *>();
        String password = doc["password"].as<const char *>();
        String id = doc["id"].as<const char *>();

        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; i++)
        {
            DynamicJsonDocument networkDoc(1024);
            networkDoc["ssid"] = WiFi.SSID(i);
            networkDoc["strength"] = WiFi.RSSI(i);
            networkDoc["encryption"] = WiFi.encryptionType(i);
            networkDoc["teamId"] = teamId;

            String response;
            serializeJson(networkDoc, response);
            SerialBT.println(response);
        }
    }
    else if (action == "connect")
    {
        String ssid = doc["ssid"];
        String password = doc["password"];

        WiFi.begin(ssid.c_str(), password.c_str());
        unsigned long startTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000)
        {
            delay(100);
        }

        DynamicJsonDocument responseDoc(1024);
        responseDoc["ssid"] = ssid;
        responseDoc["connected"] = WiFi.status() == WL_CONNECTED;
        responseDoc["teamId"] = teamId;

        String response;
        serializeJson(responseDoc, response);
        SerialBT.println(response);
    }
    else if (action == "getData" || action == "getDetails")
    {
        HTTPClient http;
        http.begin("http://proiectia.bogdanflorea.ro/api/marvel-movies/movies");
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            String payload = http.getString();
            DynamicJsonDocument responseDoc(15000);
            DeserializationError error = deserializeJson(responseDoc, payload);

            if (!error)
            {
                if (action == "getData")
                {
                    for (JsonVariant v : responseDoc.as<JsonArray>())
                    {
                        DynamicJsonDocument movieDoc(1024);
                        movieDoc["id"] = v["id"];
                        movieDoc["name"] = v["title"];
                        movieDoc["image"] = v["poster"];
                        movieDoc["teamId"] = teamId;

                        String response;
                        serializeJson(movieDoc, response);
                        SerialBT.println(response);
                    }
                }
                else
                { // getDetails
                    String id = doc["id"];
                    for (JsonVariant v : responseDoc.as<JsonArray>())
                    {
                        if (v["id"].as<String>() == id)
                        {
                            DynamicJsonDocument detailDoc(1024);
                            detailDoc["id"] = v["id"];
                            detailDoc["name"] = v["title"];
                            detailDoc["image"] = v["poster"];
                            detailDoc["description"] = v["plot"];
                            detailDoc["teamId"] = teamId;

                            String response;
                            serializeJson(detailDoc, response);
                            SerialBT.println(response);
                            break;
                        }
                    }
                }
            }
        }
        http.end();
    }
}

void receivedData()
{
    while (SerialBT.available())
    {
        data = SerialBT.readStringUntil('\n');
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);

    if (!error)
    {
        String action = doc["action"];
        handleAction(action, doc);
    }

    data = "";
}

void setup()
{
    Serial.begin(115200);
    SerialBT.begin(btcServerName);
    Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop()
{
    if (SerialBT.available())
    {
        receivedData();
    }
}
