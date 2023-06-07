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

HTTPClient http;
WiFiClient client;
bool deviceConnected = false;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

#define btcServerName "ESP_BTC_MARVEL"
#define CONNECT_TIMEOUT 15000 // ms

DynamicJsonDocument JSON_ACTION(15000);
DynamicJsonDocument JSON_RESPONSE(15000);
DynamicJsonDocument JSON_SENT(15000);
DeserializationError JSON_ERROR;

BluetoothSerial SerialBT;
volatile bool newDataReceived = false;
String receivedRequest;

//================================================
//          Helper functions
std::string getAction(std::string data)
{
    JSON_ERROR = deserializeJson(JSON_ACTION, data);
    return JSON_ACTION["action"];
};
String getId(std::string data)
{
    JSON_ERROR = deserializeJson(JSON_ACTION, data);
    return JSON_ACTION["id"].as<String>();
};
//================================================

//=========================================
//          BlueTooth - global

BluetoothSerial SerialBT;
bool connected = false;
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

String ssid = "Samsung Galaxy TBC +";
String password = "amfostlamuntesim";
const long connection_timeout = 15000; // 15s
long startConnection = 0;

//=========================================

//===========================================================================================
//              GET FIRST URL
void getNetworks(BLECharacteristic characteristic)
{
    BLECharacteristic characteristic;
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
void getData(BLECharacteristic *characteristic, String id)
{
    BLECharacteristic characteristic;
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

BLECharacteristic indexCharacteristic(
    "4defcb73-3cde-4a07-aa93-af08c579323c",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

BLEDescriptor *indexDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic detailsCharacteristic(
    "d052371d-36dc-4017-b1e6-8d11197f9e3d",
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

BLEDescriptor *detailsDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2902));

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        Serial.println(F("Device connected"));
    };
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println(F("Device disconnected"));
    }
};

//============================================================================
//                          Find Request and parse Response
void handleRequest()
{
    DynamicJsonDocument jsonDoc(15000);
    DeserializationError error = deserializeJson(jsonDoc, receivedRequest);

    if (error)
    {
        Serial.print("JSON deserialization failed: ");
        Serial.println(error.c_str());
    }

    String action = jsonDoc["action"].as<String>();
    String teamId = jsonDoc["teamId"].as<String>();

    if (action == "getNetworks")
    {

        DynamicJsonDocument responseDoc(15000);

        String response;
        serializeJson(responseDoc, response);

        SerialBT.println(response);
        Serial.println("Sent response: " + response);
    }
    else
    {
        Serial.println("Unknown action: " + action);
    }

    newDataReceived = false;
    receivedRequest = "";
}
//============================================================================

//===========================================================================================
//                                   MAKE FUNCTIONS WORK

class CharacteristicsCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string data = characteristic->getValue();
        std::string action = getAction(data);
        String id = getId(data);

        Serial.println(action.c_str());
        if (action.compare("getNetworks") == 0)
        {
            getNetworks(*characteristic);
        }
        else if (action.compare("getData") == 0)
        {
            getData(characteristic, id);
        }
    }
};
//===========================================================================================

void setup()
{
    Serial.begin(115200);

    //=========================================
    //          BlueTooth - config

    SerialBT.begin(btcServerName);
    SerialBT.register_callback(deviceConnected);
    Serial.println("Device ready for pairing!");
    //=========================================

    //=========================================
    //              WiFi - config

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
        Serial.println("Connection failed");
    else
    {
        Serial.print("Connected to network: ");
        Serial.println(WiFi.SSID());
        Serial.print("Local IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void loop()
{
    if (SerialBT.available())
    {
        String request = SerialBT.readStringUntil('\n');
        request.trim();
        handleRequest();
    }
}