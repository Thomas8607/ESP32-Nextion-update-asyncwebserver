#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "index.h"

#define sinminVal 10.0
#define sinmaxVal 90.0
#define cosminVal 0.1
#define cosmaxVal 3.8

// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);
float cosValue, sinValue;
bool data_stream;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
static unsigned long lastUpdate = millis();

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

void setup() {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
    Serial.println("AP IP címe: " + WiFi.softAPIP().toString());
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", MAIN_page);
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
}

void loop() {
    sinValue = sinminVal + ((sinmaxVal - sinminVal) / 2) + ((sinmaxVal - sinminVal) / 2) * sin(millis() * 0.001);
    cosValue = cosminVal + ((cosmaxVal - cosminVal) / 2) + ((cosmaxVal - cosminVal) / 2) * cos(millis() * 0.001);
    if (((millis() - lastUpdate) > 110) && data_stream) {
        lastUpdate = millis();
        StaticJsonDocument<500> doc;
        doc["imap"] = cosValue;
        doc["coolanttemp"] = sinValue;
        String output;
        serializeJson(doc, output);
        ws.textAll(output);
    }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        // Websocket open
        data_stream = true;
    }
    else if (type == WS_EVT_DISCONNECT) {
        // Websocket close
        data_stream = false;
        ws.cleanupClients();
    }
    else if (type == WS_EVT_DATA) {
        // Adat érkezik
    }
}