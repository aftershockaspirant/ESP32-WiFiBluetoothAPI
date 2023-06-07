#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

// BTC server name
#define btcServerName "ESP_BTC_MARVEL"

#define CONNECT_TIMEOUT 15000 // ms

String teamID = "B19";
String currentSSID = "";
bool connectedToWiFi = false;
volatile bool newDataReceived = false;
String receivedRequest;

//=========================================
//          BlueTooth - global

// Define the BluetoothSerial
BluetoothSerial SerialBT;

// Define a variable for the connection status
bool connected = false;

// Callback function for connection events
void deviceConnected(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    if (event == ESP_SPP_SRV_OPEN_EVT)
    {
        Serial.println("Device Connected");
        connected = true;
    }
    if (event == ESP_SPP_CLOSE_EVT)
    {
        Serial.println("Device disconnected");
        connected = false;
    }
}

//=========================================

//=========================================
//              WiFi - global

// Define WiFi credentials
// String ssid = "Samsung Galaxy TBC +";
// String password = "amfostlamuntesim";
const long connection_timeout = 15000; // 15s
long startConnection = 0;

//=========================================

//===========================================================================================
//              GET FIRST URL
void getData()
{
    // BLECharacteristic characteristic;
    if (WiFi.status() == WL_CONNECTED)
    {

        String url = "http://proiectia.bogdanflorea.ro/api/marvel-movies/movies";

        http.begin(url);
        http.setConnectTimeout(30000);
        http.setTimeout(30000);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);

            //=========================================
            //            Process payload

            DynamicJsonDocument jsonDoc(15000);
            DeserializationError error = deserializeJson(jsonDoc, payload);
            http.end();

            if (error)
            {
                Serial.print("JSON deserialization failed: ");
                Serial.println(error.c_str());
            }
            else
            {
                JsonArray list = jsonDoc.as<JsonArray>();
                int i = 1;
                for (JsonVariant value : list)
                {
                    JsonObject listItem = value.as<JsonObject>();

                    Serial.print(i);
                    Serial.print(". ");
                    Serial.print(listItem["id"].as<String>());
                    Serial.print(" (");
                    Serial.print(listItem["title"].as<String>());
                    Serial.print("): ");
                    Serial.print(listItem["cover_url"].as<String>());

                    Serial.println("----------------------------------------------------------");
                    const size_t CAPACITY = JSON_OBJECT_SIZE(4096);
                    DynamicJsonDocument responseDocument(CAPACITY);
                    JsonObject object = responseDocument.to<JsonObject>();
                    object.set(listItem);
                    String responseString;

                    serializeJson(responseDocument, responseString);
                    Serial.println(responseString);

                    delay(100);
                    i++;
                }
            } //=========================================
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
    }
}
//===========================================================================================

//===========================================================================================
//            GET URL DETAILS
void getDetails(String id)
{
    // BLECharacteristic characteristic;
    if (WiFi.status() == WL_CONNECTED)
    {

        String url = "http://proiectia.bogdanflorea.ro/api/marvel-movies/movies" + id;

        http.begin(url);
        http.setConnectTimeout(30000);
        http.setTimeout(30000);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);

            //=========================================
            //            Process payload

            DynamicJsonDocument jsonDoc(15000);
            DeserializationError error = deserializeJson(jsonDoc, payload);
            http.end();

            if (error)
            {
                Serial.print("JSON deserialization failed: ");
                Serial.println(error.c_str());
            }
            else
            {
                JsonArray list = jsonDoc.as<JsonArray>();
                int i = 1;
                for (JsonVariant value : list)
                {
                    JsonObject listItem = value.as<JsonObject>();

                    Serial.print(i);
                    Serial.print(". ");
                    Serial.print(listItem["id"].as<String>());
                    Serial.print(" (");
                    Serial.print(listItem["title"].as<String>());
                    Serial.print("): ");
                    Serial.print(listItem["release_date"].as<String>());
                    Serial.print(listItem["box_office"].as<String>());
                    Serial.print(listItem["duration"].as<String>());
                    Serial.print(listItem["overview"].as<String>());
                    Serial.print(listItem["cover_url"].as<String>());
                    Serial.print(listItem["trailer_url"].as<String>());
                    Serial.print(listItem["directed_by"].as<String>());
                    Serial.print(listItem["phase"].as<String>());
                    Serial.print(listItem["saga"].as<String>());
                    Serial.print(listItem["chronology"].as<String>());
                    Serial.print(listItem["post_credit_scenes"].as<String>());
                    Serial.print(listItem["imdb_id"].as<String>());

                    Serial.println("----------------------------------------------------------");
                    const size_t CAPACITY = JSON_OBJECT_SIZE(15000);
                    DynamicJsonDocument responseDocument(CAPACITY);
                    JsonObject object = responseDocument.to<JsonObject>();
                    object.set(listItem);
                    String responseString;

                    serializeJson(responseDocument, responseString);
                    Serial.println(responseString);

                    delay(100);
                    i++;
                }
            } //=========================================
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
        }
    }
}
//===========================================================================================

void parseRequest(String request)
{

    DynamicJsonDocument jsonDoc(1024);
    DeserializationError error = deserializeJson(jsonDoc, receivedRequest);

    if (error)
    {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
    }

    // Extract the action and teamId values from the request
    String action = jsonDoc["action"].as<String>();
    String teamId = jsonDoc["teamId"].as<String>();

    // Check the action value and perform the corresponding action
    if (action == "getNetworks" && teamId == teamID)
    {
        getNetworks();
    }

    else if (action == "connect")
    {

        String ssid = jsonDoc["ssid"].as<String>();
        String password = jsonDoc["password"].as<String>();
        connect(ssid, password);
    }

    else if (action == "getData")
    {
        getData();
    }
    else if (action == "getDetails")
    {
        String id = jsonDoc["id"].as<String>();
        getDetails(id);
    }
    else
    {
        Serial.println("Unknown action: " + action);
    }

    newDataReceived = false;
    receivedRequest = "";
}

void getNetworks()
{
    Serial.println("Start scan");
    int networksFound = WiFi.scanNetworks();
    Serial.println("Scan complete");
    if (networksFound == 0)
    {
        Serial.println("No networks found");
    }
    else
    {
        Serial.print(networksFound);
        Serial.println(" networks found");
        DynamicJsonDocument jsonDoc(15000);
        DeserializationError error = deserializeJson(jsonDoc, networksFound);
        http.end();

        if (error)
        {
            Serial.print("JSON deserialization failed: ");
            Serial.println(error.c_str());
        }
        else
        {
            JsonArray list = jsonDoc.as<JsonArray>();
            int i = 1;
            for (JsonVariant value : list)
            {
                JsonObject listItem = value.as<JsonObject>();

                Serial.print(i);
                Serial.print(". ");
                Serial.print(listItem["ssid"].as<String>());
                Serial.print(listItem["strength"].as<float>());
                Serial.print(listItem["encryption"].as<String>());
                Serial.print(listItem["teamId"].as<String>());
                Serial.println("----------------------------------------------------------");

                const size_t CAPACITY = JSON_OBJECT_SIZE(15000);
                DynamicJsonDocument responseDocument(CAPACITY);
                JsonObject object = responseDocument.to<JsonObject>();
                object.set(listItem);
                String responseString;

                serializeJson(responseDocument, responseString);
                Serial.println(responseString);

                SerialBT.println(responseString);
                Serial.println("Sent responseString: " + responseString);
                delay(100);
                i++;
            }
        }
    }
}

void connect(const String &ssid, const String &password)
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting");
    startConnection = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
        if (millis() - startConnection > connection_timeout)
        {
            break;
        }
    }

    Serial.println("");
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connection failed");
        currentSSID = "";
        connectedToWiFi = false;
    }
    else
    {
        Serial.println("Connected to WiFi!");
        currentSSID = ssid;
        connectedToWiFi = true;

        DynamicJsonDocument responseDoc(1024);
        responseDoc["ssid"] = currentSSID;
        responseDoc["connected"] = connectedToWiFi;
        responseDoc["teamId"] = teamID;

        String responseString;
        serializeJson(responseDoc, responseString);

        SerialBT.println(responseString);
        Serial.println("Sent response: " + responseString);
    }
}

void setup()
{
    Serial.begin(115200);

    //=========================================
    //          BlueTooth - config

    // Initialize BTC
    SerialBT.begin(btcServerName);

    // Register the callback function for connect/disconnect events
    SerialBT.register_callback(deviceConnected);
    Serial.println("Device ready for pairing!");
    //=========================================

    //=========================================
    //              WiFi - config

    // Set WiFi to station mode and disconnect from an AP if it
    // was previously connected
}

void loop()
{
    if (newDataReceived)
    {
        parseRequest(request);
    }
    delay(5000);
}