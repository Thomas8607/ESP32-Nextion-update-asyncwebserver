#include <ESPAsyncWebServer.h>
//#include <Update.h>
//#include <NextionUploadWIFI.h>
#include "esp_pages.h"
#include "nextion_pages.h"
#include "index_page.h"
#include "charts_pages.h"

#define CHECK_STATUS_1 "Baudrate kiolvasás nem sikerült!"
#define CHECK_STATUS_2 "Baudrate megváltoztatás nem sikerült!"
#define UPLOAD_STATUS "Kapcsolat megszakadt!"
#define SERIAL2_BAUD 115200
#define SERIAL2_RX_PIN GPIO_NUM_41
#define SERIAL2_TX_PIN GPIO_NUM_42
#define SERVER_PORT 80
#define INTERVAL 80
// Websocket struktúra
struct Data {
    uint32_t time;
    uint16_t rpm;
    int16_t coolantTemp;
    int16_t imapPressure;
    int16_t emapPressure;
    int16_t intakeairTemp;
    int16_t acceleration; 
};

// Ideiglenes adatok
#define sinminVal 10.0
#define sinmaxVal 90.0
#define cosminVal 0.1
#define cosmaxVal 3.8
float coolant, imap, emap, intakeair, acceleration;
uint16_t rpm;

// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);
String FW_VERSION = "2.0";
bool espShouldReboot;
float cosValue, sinValue;
static unsigned long lastUpdate = millis();
bool data_stream;
uint32_t filesize;
uint8_t check_status;
bool upload_status;
String check_reason = "";
String error_reason = "";
bool nextionShouldReboot;

AsyncWebServer server(SERVER_PORT);
AsyncWebSocket ws("/ws");
// Create Nextion WiFi Uploader object
//NextionUploadWIFI nextion(SERIAL2_BAUD, SERIAL2_RX_PIN, SERIAL2_TX_PIN);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void WebsocketSending(const uint32_t interval, bool dataStream, uint16_t RpmData, float CoolantData, float ImapData, float EmapData, float IntakeairData, float AccelerationData);
//**************************************************************************************************************************************************************************
void setup() {
    // Serial port inicializálása
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
    //Serial.println("AP IP címe: " + WiFi.softAPIP().toString());
//**************************************************************VÁLASZTÓ OLDAL********************************************************************************************
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
//*********************************************ESP UPDATE*****************************************************************************************************************
	// Ha beírjuk az IP címet /esp
	server.on("/esp", HTTP_GET, [](AsyncWebServerRequest *request) {
		String view_html;
		view_html += esp_header_html;
		view_html += "<label><h3>Aktuális firmware verzió: " + FW_VERSION + "</h3></label>";
		view_html += esp_update_html;
		request->send(200, "text/html", view_html);
	});
    /*
	// Ha a frissítés gombot megnyomjuk
	server.on("/esp_update", HTTP_POST, [](AsyncWebServerRequest *request) {
  	espShouldReboot = !Update.hasError();
  	AsyncWebServerResponse *response = request->beginResponse(200, "text/html", espShouldReboot ? esp_update_success_html : esp_update_failed_html);
  	response->addHeader("Connection", "close");
  	request->send(response); }, 
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
			//Serial.printf("Start update: %s\n", filename.c_str());
			if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
				Update.printError(Serial);
			}
		}
		if (!Update.hasError()) {
			if (Update.write(data, len) != len) {
				Update.printError(Serial);
			}
		}
		if (final) {
			if (Update.end(true)) {
				//Serial.printf("Update was successfull: %uB\n", index + len);
			}
			else {
				Update.printError(Serial);
			}
		} 
	});
    */
//***********************************************NEXTION UPDATE************************************************************************************************* 
// Index page
    server.on("/nextion", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", nextion_index_html); 
    });
// Fail page
    server.on("/nextion_fail", HTTP_GET, [](AsyncWebServerRequest *request) {
		String view_html;
		view_html += nextion_update_failed_header_html;
		view_html += "<label><h3>Hiba oka: " + error_reason + "</h3></label>";
		view_html += nextion_update_failed_footer_html;
        request->send(302, "text/html", view_html);
    });
// Success page
    server.on("/nextion_success", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(302, "text/html", nextion_update_success_html);
    });
// Receive Firmware file size
    server.on("/size", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            filesize = atoi((const char *)data);
            /*
            check_status = nextion.check(filesize);
            if (check_status == 1) {
                error_reason = CHECK_STATUS_1;
                request->redirect("/nextion_fail");
            }
            if (check_status == 2) {
                error_reason = CHECK_STATUS_2;
                request->redirect("/nextion_fail");
            }
            if (check_status == 0) {
                error_reason = "";
                request->send(200);
            }
            */
    });
    // Receive Firmware cunks and flash Nextion display
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
        if(len) {
            //upload_status = nextion.uploadTftFile(data, len);
            if (!upload_status) {
                error_reason = UPLOAD_STATUS;
                request->redirect("/nextion_fail");
            }
            else {
                request->send(200);
            }
        }
    });
//***************************************GRAFIKON***********************************************//
    server.on("/grafikon", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", grafikon_html);
    });
    server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/javascript", highcharts_js);
    });
    server.on("/exporting.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/javascript", exporting_js);
    });
    server.on("/offline-exporting.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/javascript", offline_exporting_js);
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
    //server.end();
}
// Loop függvény
void loop() {
	if (espShouldReboot) {
		delay(100);
		ESP.restart();
	}
// Ideiglenes adatok
    coolant = sinminVal + ((sinmaxVal - sinminVal) / 2) + ((sinmaxVal - sinminVal) / 2) * sin(millis() * 0.001);
    imap = cosminVal + ((cosmaxVal - cosminVal) / 2) + ((cosmaxVal - cosminVal) / 2) * cos(millis() * 0.001);
    emap = -1 * imap;
    intakeair = 125.2;
    acceleration = -1.66;
    rpm = 3500;

    WebsocketSending(INTERVAL, data_stream, rpm, coolant, imap, emap, intakeair, acceleration);
}


void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        data_stream = true;
    }
    else if (type == WS_EVT_DISCONNECT) {
        data_stream = false;
        ws.cleanupClients();
    }
    else if (type == WS_EVT_DATA) {
    }
}

void WebsocketSending(const uint32_t interval, bool dataStream, uint16_t RpmData, float CoolantData, float ImapData, float EmapData, float IntakeairData, float AccelerationData) {
    static uint32_t lastUpdate = 0;
    static struct Data wsData;
    if(!dataStream) {
        wsData.time = 0;   
    }
    if (((millis() - lastUpdate) > interval) && dataStream) {
        wsData.coolantTemp = int16_t((CoolantData) * 10);
        wsData.imapPressure = int16_t((ImapData) * 100);
        wsData.emapPressure = int16_t((EmapData) * 100);
        wsData.intakeairTemp = int16_t((IntakeairData) * 10);
        wsData.rpm = uint16_t(RpmData);
        wsData.acceleration = int16_t((AccelerationData) * 100);
        wsData.time += INTERVAL;
        uint8_t * bytePtr = (uint8_t*) &wsData;    
        ws.binaryAll(bytePtr, sizeof(wsData));
        lastUpdate = millis();
    }
}