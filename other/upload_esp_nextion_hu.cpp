#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <NextionUploadWIFI.h>


#define CHECK_STATUS_1 "Baudrate kiolvasás nem sikerült!"
#define CHECK_STATUS_2 "Baudrate megváltoztatás nem sikerült!"
#define UPLOAD_STATUS "Kapcsolat megszakadt!"
#define SERIAL2_BAUD 115200
#define SERIAL2_RX_PIN GPIO_NUM_41
#define SERIAL2_TX_PIN GPIO_NUM_42
#define SERVER_PORT 80

// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);
String FW_VERSION = "2.0";
bool espShouldReboot;
AsyncWebServer server(SERVER_PORT);
// Create Nextion WiFi Uploader object
NextionUploadWIFI nextion(SERIAL2_BAUD, SERIAL2_RX_PIN, SERIAL2_TX_PIN);


uint32_t filesize;
uint8_t check_status;
bool upload_status;
String check_reason = "";
String error_reason = "";
bool nextionShouldReboot;


void notFoundResponse(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
//*********************************************************************KIVÁLASZTÓ MENÜ***********************
const char* index_html PROGMEM = R"====(
<!DOCTYPE html>
    <html lang="en">
    <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Autóműszerfal web menü</title>
    <style>
      body {
        background:MediumSeaGreen;
      }
      form {
        background: Gray;
        max-width: 500px;
        margin: 50px auto;
        padding: 30px;
        border-radius: 25px;
        text-align: center 
      }
      .btn1 {
        background: LightGreen;
        width: 100%;
        height: 100px;
        border-radius: 20px;
        font-size:20px;
      }
      .btn2 {
        background: LightBlue;
        width: 100%;
        height: 100px;
        border-radius: 20px;
        font-size:20px;
      }
      .btn3 {
        background: LightGray;
        width: 100%;
        height: 100px;
        border-radius: 20px;
        font-size:20px;
      }
    </style>
</head>
<body>
    <form>
      <label><b><h1>Funkció kiválasztás</h1></b></label><br>
      <input type="button" class="btn1" value="ESP32 processzor firmware frissítés" onclick="window.location.href='/esp'"/><br><br><br><br>
      <input type="button" class="btn2" value="Kijelző firmware frissítés" onclick="window.location.href='/nextion'"/><br><br><br><br>
    </form>
</body>
</html>
)====";
//*************************************************************************ESP32 FRISSITO OLDALAK*************************************************************************************
// ESP32 frissítő oldal első része
const char* esp_header_html PROGMEM = R"====(
<!DOCTYPE html>
  <html lang="en">
    <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 processzor frissítő</title>
      <style>
        body {
          background: DodgerBlue;
          font-family: sans-serif;
        }
        form {
          background: white;
          max-width: 450px;
          margin: 50px auto;
          padding: 100px;
          border-radius: 25px;
          text-align: center
        }
        input[type="submit"] {
          background-color: Green;
          padding: 10px 20px;
          border-radius: 8px;
        }
      </style>
      <script>
        function valSubmit() {
            var file = document.querySelector("input[name=update]").files[0];
            if (!file) {
                alert("Válassz fájlt!");
                return false;
            } 
            else if (!file.name.endsWith(".bin")) {
                alert("Helytelen fájl kiterjesztés. Csak .bin fájl megengedett!");
                var fileInput = document.querySelector("input[name=update]");
                fileInput.value = null;
                return false;
            }
            return true;
        }
    </script>
  </head>
  <body>
    <form method='POST' action='/esp_update' enctype='multipart/form-data' onsubmit='return valSubmit()'>
      <label><b><h1>ESP32 processzor frissítő</h1></b></label>
)====";
// ESP32 frissítő oldal Verziószám utáni része
const char* esp_update_html PROGMEM = R"====(
      <input accept='.bin' type='file' name='update'>
      <h5>Csak .bin fájl engedélyezett</h5>
      <input type='submit' value='Frissítés kezdése'>
    </form>
  </body>
</html>
)====";
// ESP32 frissítés sikeres oldal
const char* esp_update_success_html PROGMEM = R"====(
<!DOCTYPE html>
  <html>
    <head>
      <meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'>
      <title>ESP32 processzor frissítő</title>
      <style>
        body {
          background: DodgerBlue;
        }
        form {
          background: Green;
          max-width: 500px;
          margin: 50px auto;
          padding: 30px;
          border-radius: 25px;
          text-align: center
        }
	      .btn {
          padding: 10px 40px;
          border-radius: 10px;
        }
      </style>
    </head>
    <body>
      <form>
        <h1><strong>Sikeres frissítés!</strong></h1>
        <h3>Az ESP32 újraindul!</h3>
        <input type="button" class="btn" value="Vissza" onclick="window.location.href='/esp'"/>
      </form>
    </body>
  </html>
)====";
// ESP32 frissítés sikertelen oldal
const char* esp_update_failed_html PROGMEM = R"====(
<!DOCTYPE html>
  <html>
    <head>
    <meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'>
    <title>ESP32 processzor frissítő</title>
    <style>
      body {
        background: DodgerBlue;
      }
      form {
        background: Red;
        max-width: 500px;
        margin: 50px auto;
        padding: 30px;
        border-radius: 25px;
        text-align: center
      }
      .btn {
        padding: 10px 40px;
        border-radius: 10px;
      }
      </style>
    </head>
    <body>
      <form>
        <h1><strong>Sikertelen frissítés!</strong></h1>
        <input type="button" class="btn" value="Vissza" onclick="window.location.href='/esp'"/>
      </form>
    </body>
  </html>
)====";
//*******************************************************NEXTION FRISSÍTŐ OLDALAK******************************************************************************************
// Nextion update index page
const char *nextion_index_html PROGMEM = R"====(
<!DOCTYPE html>
    <html lang="en">
        <head>
            <meta http-equiv="content-type" content="text/html; charset=utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <title>Nextion kijelző frissítő</title>
            <style>
                body {
                    background: DodgerBlue;
                    font-family: sans-serif;
                }
                form {
                    background: white;
                    max-width: 450px;
                    margin: 50px auto;
                    padding: 100px;
                    border-radius: 25px;
                    text-align: center
                }
                    input[type="button"] {
                    background-color: Green;
                    padding: 10px 20px;
                    border-radius: 8px;
                    color: black;
                }
        </style>
            <script>
            var partSize = 100;
            var file;
            var cmp;
            var offset = 0;
            function valCheck() {
                file = document.querySelector("input[name=file]").files[0];
                partSize = parseInt(document.querySelector("input[name=partSize]").value);
                cmp = document.getElementById("completed");
                if (file) {
                    if (file.name.endsWith(".tft")) {
                        var xhttp = new XMLHttpRequest();
                        xhttp.onreadystatechange = function(){
                        if(xhttp.readyState == 4 && xhttp.status == 200) {
                            document.getElementById("button").disabled = false;
                        } 
                        if(xhttp.readyState == 4 && xhttp.status == 302) {
                            window.location.href = "/nextion_fail";
                        }
                    };
                    sendInfo(xhttp, file.size);
                    } else {
                        alert("Helytelen fájl kiterjesztés. Csak .tft fájl megengedett!");
                        var fileInput = document.querySelector("input[name=file]");
                        fileInput.value = null;
                    }
                } else {
                    alert("Válassz fájlt!");
                }
            }
            function sendInfo(xmlHttp, size) {
                xmlHttp.open("post", "/size");
                xmlHttp.send(size);
            }
            function sendDataHandler(event) {
                if (event.target.error == null) {
                    cmp.innerText = (offset * 100 / file.size).toFixed(0) + "%";
                    offset += event.target.result.byteLength;
                } else {
                    alert("Error: " + event.target.error);
                    return;
                }
                var xmlHttp = new XMLHttpRequest();
                xmlHttp.onreadystatechange = function() {
                    if (xmlHttp.readyState == 4 && xmlHttp.status == 200) {
                        if (offset < file.size) {
                            sendData();
                        } else {
                            window.location.href = "/nextion_success";
                            cmp.innerText = "All data was sent";
                        }
                    }
                    if ((xmlHttp.status == 302) || (xmlHttp.status == 404)) {
                        window.location.href = "/nextion_fail";
                    }
                };        
                xmlHttp.open("post", "/update");
                xmlHttp.send(event.target.result);
            }
            function sendData() {
                document.getElementById("button").disabled = true;
                document.getElementById("uploading").style.display = "inline";
                var reader = new FileReader();
                var blob = file.slice(offset, partSize + offset);
                reader.onload = sendDataHandler;
                reader.readAsArrayBuffer(blob);
            }
            </script>
        </head>
    <body>
        <form>
            <label><b><h1>Nextion kijelző frissítő</h1></b></label><br>
            <input type="file" name="file" onchange="valCheck()"><br>
            <h5>Csak .tft fájl engedélyezett</h5>
            <input type="button" id="button" value="Frissítés kezdése" onclick="sendData()" disabled><br><br><br>
            <span id="uploading" style="display:none;">Frissítés folyamatban...</span><br>
	          <br><label id="completed"></label><br><br>
            Darabolási méret: <input type="text" name="partSize" value="1024" size="4">
        </form>    
    </body>
</html>
)====";

// Nextion update error header
const char *nextion_update_failed_header_html PROGMEM = R"====(
<!DOCTYPE html>
 <html lang="en">
    <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Nextion kijelző frissítő</title>
    <style>
      body {
        background: DodgerBlue;
      }
      form {
        background: Red;
        max-width: 500px;
        margin: 50px auto;
        padding: 30px;
        border-radius: 25px;
        text-align: center
      }
      .btn {
        padding: 10px 40px;
        border-radius: 10px;
      }
      </style>
    </head>
    <body>
      <form>
      <label><h1>Sikertelen frissítés</h1></label>
)====";

const char *nextion_update_failed_footer_html PROGMEM = R"====(
        <input type="button" class="btn" value="Vissza" onclick="window.location.href='/nextion'">
      </form>
    </body>
  </html>
)====";
// Nextion update success
const char *nextion_update_success_html PROGMEM = R"====(
<!DOCTYPE html>
  <html lang="en">
    <head>
      <meta http-equiv="content-type" content="text/html; charset=utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>Nextion kijelző frissítő</title>
      <style>
        body {
          background: DodgerBlue;
        }
        form {
          background: Green;
          max-width: 500px;
          margin: 50px auto;
          padding: 30px;
          border-radius: 25px;
          text-align: center
        }
	      .btn {
          padding: 10px 40px;
          border-radius: 10px;
        }
      </style>
    </head>
    <body>
      <form>
        <h1><strong>Sikeres frissítés!</strong></h1>
        <br>
        <br>
        <h3>A kijelző újraindul!</h3>
        <input type="button" class="btn" value="Vissza" onclick="window.location.href='/'">
      </form>
    </body>
  </html>
)====";
//**************************************************************************************************************************************************************************
void setup() {
    // Serial port inicializálása
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
    Serial.println("AP IP címe: " + WiFi.softAPIP().toString());
//**************************************************************VÁLASZTÓ OLDAL********************************************************************************************
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
//*********************************************ESP UPDATE*********************************************************************************
	// Ha beírjuk az IP címet /esp
	server.on("/esp", HTTP_GET, [](AsyncWebServerRequest *request) {
		String view_html;
		view_html += esp_header_html;
		view_html += "<label><h3>Aktuális firmware verzió: " + FW_VERSION + "</h3></label>";
		view_html += esp_update_html;
		request->send(200, "text/html", view_html);
	});
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
//***********************************************NEXTION UPDATE************************************************************************************************* 
// Index page
    server.on("/nextion", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(nextion_index_html);
        request->send(response); 
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
        request->send(302, "text/html", nextion_update_success_html);
    });
// Receive Firmware file size
    server.on("/size", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            filesize = atoi((const char *)data);
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
    });

    // Receive Firmware cunks and flash Nextion display
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
        if(len) {
            upload_status = nextion.uploadTftFile(data, len);
            if (!upload_status) {
                error_reason = UPLOAD_STATUS;
                request->redirect("/nextion_fail");
            }
            else {
                request->send(200);
            }
        }
    });
  // NotFound
  server.onNotFound(notFoundResponse);
  // Szerver indítása
  server.begin();
  // server.end();
}

// Loop függvény
void loop()
{
	if (espShouldReboot)
	{
		// Serial.println(F("Esp ujraindul ..."));
		delay(100);
		ESP.restart();
	}
}