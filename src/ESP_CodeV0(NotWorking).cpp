#include <ArduinoJson.h>
#include <BluetoothSerial.h>
#include <HTTPClient.h>
#include <WiFi.h>

BluetoothSerial SerialBT;
String teamId;
const char *apiUrl = "http://proiectia.bogdanflorea.ro/api/marvel-movies/movies";

void getNetworks();
void connect(String ssid, String password);
void getData();
void getDetails(String id);

void setup()
{
    Serial.begin(115200);
    SerialBT.begin("ESP32_MarvelMovies");
    WiFi.mode(WIFI_AP);
    getNetworks();
}

void loop()
{
    if (SerialBT.available())
    {
        String request = SerialBT.readString();
        DynamicJsonDocument jsonDoc(1024);
        deserializeJson(jsonDoc, request);

        String action = jsonDoc["action"];

        if (action == "getNetworks")
        {
            teamId = jsonDoc["teamId"].as<String>();
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
    }
}

void getNetworks()
{
    int numNetworks = WiFi.scanNetworks();
    DynamicJsonDocument response(4096);
    JsonArray networks = response.to<JsonArray>();

    for (int i = 0; i < numNetworks; i++)
    {
        JsonObject network = networks.createNestedObject();
        network["ssid"] = WiFi.SSID(i);
        network["strength"] = WiFi.RSSI(i);
        network["encryption"] = WiFi.encryptionType(i);
        network["teamId"] = teamId;
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    SerialBT.print(jsonResponse);
}

void connect(String ssid, String password)
{
    WiFi.begin(ssid.c_str(), password.c_str());
    int startTime = millis();
    bool connected = false;

    while (millis() - startTime < 15000)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            connected = true;
            break;
        }
        delay(100);
    }

    DynamicJsonDocument response(256);
    response["ssid"] = ssid;
    response["connected"] = connected;
    response["teamId"] = teamId;

    String jsonResponse;
    serializeJson(response, jsonResponse);
    SerialBT.print(jsonResponse);
}

void getData()
{
    HTTPClient http;
    http.begin(apiUrl);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        DynamicJsonDocument jsonDoc(4096);
        deserializeJson(jsonDoc, payload);
        JsonArray jsonArray = jsonDoc.as<JsonArray>();

        DynamicJsonDocument response(4096);
        JsonArray responseData = response.to<JsonArray>();

        for (JsonObject item : jsonArray)
        {
            JsonObject movie = responseData.createNestedObject();
            movie["id"] = item["id"].as<String>();
            movie["name"] = item["name"].as<String>();
            movie["image"] = item["thumbnail"].as<String>();
            movie["teamId"] = teamId;
        }

        String jsonResponse;
        serializeJson(response, jsonResponse);
        SerialBT.print(jsonResponse);
    }
    http.end();
}

void getDetails(String id)
{
    String movieUrl = String(apiUrl) + "/" + id;
    HTTPClient http;
    http.begin(movieUrl);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        String payload = http.getString();
        DynamicJsonDocument jsonDoc(1024);
        deserializeJson(jsonDoc, payload);
        JsonObject jsonObject = jsonDoc.as<JsonObject>();

        DynamicJsonDocument response(1024);
        response["id"] = jsonObject["id"].as<String>();
        response["name"] = jsonObject["name"].as<String>();
        response["image"] = jsonObject["thumbnail"].as<String>();

        String description = "Title: " + jsonObject["name"].as<String>() + "\n";
        description += "Year: " + jsonObject["year"].as<String>() + "\n";
        description += "IMDb Rating: " + jsonObject["imdb_rating"].as<String>() + "\n";
        description += "Director: " + jsonObject["director"].as<String>() + "\n";
        description += "Duration: " + jsonObject["duration"].as<String>() + "\n";

        response["description"] = description;
        response["teamId"] = teamId;

        String jsonResponse;
        serializeJson(response, jsonResponse);
        SerialBT.print(jsonResponse);
    }
    http.end();
}