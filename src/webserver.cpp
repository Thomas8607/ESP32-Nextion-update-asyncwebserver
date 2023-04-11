#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "index.h" // HTML webpage contents with javascripts


#define ledPin GPIO_NUM_38

// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);

bool g_ledStatus(LOW);


StaticJsonDocument<100> g_doc;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static unsigned long g_lastUpdate = millis();

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void changeLed();

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
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
    if ((millis() - g_lastUpdate) > 10) {
        g_lastUpdate = millis();

        g_doc["illuminance"] = 25.99;
        g_doc["temperature"] = 59.12;

        String output;
        serializeJson(g_doc, output);
        ws.textAll(output);
        ws.cleanupClients();
    }
}


void changeLed() {
    g_ledStatus = !g_ledStatus;
    if (g_ledStatus)
        digitalWrite(ledPin, HIGH);
    else
        digitalWrite(ledPin, LOW);

    g_doc["ledStatus"] = g_ledStatus;
    String output;
    serializeJson(g_doc, output);
    ws.textAll(output);
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("Websocket client connection received");
    }
    else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
    else if (type == WS_EVT_DATA) {
        if ((char)data[0] == 'C') {
            changeLed();
        }
        else {
            Serial.print("Data received: ");
            for (int i = 0; i < len; i++) {
                Serial.print((char)data[i]);
            }
            Serial.println();
        }
    }
}
