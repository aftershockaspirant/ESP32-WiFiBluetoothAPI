#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

// BTC server name
#define btcServerName "ESP_BTC_MARVEL"

#define CONNECT_TIMEOUT 15000 // ms

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
String ssid = "Samsung Galaxy TBC +";
String password = "amfostlamuntesim";
const long connection_timeout = 15000; // 15s
long startConnection = 0;

//=========================================

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
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Initialize the WiFi network
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Connecting");

  // Connect to the network
  // Check the connection status in a loop every 0.5 seconds
  // Since the connection can take some time and might fail,
  // it is necessary to check the connection status before
  // proceeding. To define a timeout for the action, the
  // current counter of the timer is stored
  startConnection = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    // Check if the defined timeout is exceeded
    if (millis() - startConnection > connection_timeout)
    {
      break;
    }
  }

  Serial.println("");
  // Check if connection was successful
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connection failed");
  }
  else
  {
    Serial.print("Connected to network: ");
    Serial.println(WiFi.SSID());
    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());
    // Define the HTTP client
    HTTPClient http;
    // URL
    String url = "http://proiectia.bogdanflorea.ro/api/marvel-movies/movies";
    // Define the connection parameters and, optionally,
    // set the connection timeout, especially if accessing
    // a server with a longer response time (such as a
    // free API server)
    http.begin(url);
    http.setConnectTimeout(30000); // connect timeout
    http.setTimeout(30000);        // response timeout
    // Send HTTP request
    // In this exemple, the GET method is used, according to
    // the API docs
    int httpResponseCode = http.GET();
    // Check response code
    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      // Get response data
      String payload = http.getString();
      Serial.println(payload);

      //=========================================
      //         TODO – Process payload
      // Choose a static or dynamic JSON document, depending
      // on the size of the JSON data (see docs for details).
      DynamicJsonDocument jsonDoc(15000);
      DeserializationError error = deserializeJson(jsonDoc, payload);
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
          // Get the current item in the iterated list as a JsonObject
          JsonObject listItem = value.as<JsonObject>();
          // Process the data
          // This example will simply print the array information as follows:
          // Index. Name (Gender): Occupation
          // Details
          Serial.print(i);
          Serial.print(". ");
          Serial.print(listItem["id"].as<String>());
          Serial.print(" (");
          Serial.print(listItem["title"].as<String>());
          Serial.print("): ");
          Serial.println(listItem["cover_url"].as<String>());

          Serial.println("----------------------------------------------------------");
          // Encode the listItem object as a JSON string (see the library docs)
          // Check the maximum transfer unit capacity for Bluetooth low energy
          const size_t CAPACITY = JSON_OBJECT_SIZE(15000);
          DynamicJsonDocument responseDocument(CAPACITY);
          JsonObject object = responseDocument.to<JsonObject>();
          // Here the list item is simply copied to the output document
          object.set(listItem);
          // the responseString will store the endoed JSON object
          String responseString;
          serializeJson(responseDocument, responseString);
          Serial.println(responseString);
          // Copy and paste the printed JSON strings to the following website to check their size
          // https://www.debugbear.com/json-size-analyzer
          // Add a small delay (100-200ms)
          delay(100);
          i++;
        }
      }
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    // Free resources - terminate the http object (IMPORTANT)
    http.end();
  }
}

void loop()
{
  // // Scan for WiFi networks
  // Serial.println("Start scan");
  // int networksFound = WiFi.scanNetworks();
  // Serial.println("Scan complete");
  // if (networksFound == 0)
  // {
  //   Serial.println("No networks found");
  // }
  // else
  // {
  //   Serial.print(networksFound);
  //   Serial.println(" networks found");
  //   // Iterate the network list and display the information for
  //   // each network
  //   for (int i = 0; i < networksFound; i++)
  //   {
  //     bool open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
  //     Serial.print(i + 1);
  //     Serial.print(". ");
  //     Serial.print(WiFi.SSID(i));
  //     Serial.print(" (");
  //     Serial.print(WiFi.RSSI(i));
  //     Serial.print("dB)");
  //     Serial.print(" - ");
  //     Serial.println((open) ? "Open" : "Protected");
  //     delay(10);
  //   }
  // }

  // // Wait 5 seconds
  // delay(5000);
}