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
uint32_t time_now; 
uint32_t period = 100;
float vizhofok;
float imap_nyomas;
float gyorsulas;

// Vízhőfok string átalakítás
String VizhofoktoString() {
  if (isnan(vizhofok)) {    
    return "";
  }
  else {
    return String(vizhofok);
  }
}
// Imap string átalakítás
String ImaptoString() {
  if (isnan(imap_nyomas)) {
    return "";
  }
  else {
    return String(imap_nyomas);
  }
}
// Gyorsulás string átalakítás
String GyorsulastoString() {
  if (isnan(gyorsulas)) {
    return "";
  }
  else {
    return String(gyorsulas);
  }
}

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
      <input type="button" class="btn3" value="Grafikon kijelzés" onclick="window.location.href='/grafikon'"/>
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
//*************************************************GRAFIKONOK*********************************************
const char* grafikon_html PROGMEM = R"====(
<!DOCTYPE HTML>
<html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1' meta charset='UTF-8'>
    <script src='https://code.highcharts.com/highcharts.js'></script>
    <script src='https://code.highcharts.com/modules/exporting.js'></script>
    <script src='https://code.highcharts.com/modules/export-data.js'></script>
    <style>
      body {
        min-width: 310px;
        max-width: 1000px;
        height: 400px;
        margin: 0 auto;
      }
      h2 {
        font-family: Arial;
        font-size: 2.5rem;
        text-align: center;
      }
    </style>
  </head>
  <body>
    <h2>Autóműszerfal adatok</h2>
    <div id='chart-Vizhofok' class='container'></div>
    <div>Minimum érték: <span id='min-Vizhofok'></span></div>
    <div>Maximum érték: <span id='max-Vizhofok'></span>
    <button id='reset-button-Vizhofok'>Törlés</button></div>
    <div id='chart-Imap' class='container'></div>
    <div>Minimum érték: <span id='min-Imap'></span></div>
    <div>Maximum érték: <span id='max-Imap'></span>
    <button id='reset-button-Imap'>Törlés</button></div>
    <div id='chart-Gyorsulas' class='container'></div>
    <div>Minimum érték: <span id='min-Gyorsulas'></span></div>
    <div>Maximum érték: <span id='max-Gyorsulas'></span>
    <button id='reset-button-Gyorsulas'>Törlés</button></div>
  </body>

  
  <script>
    var chartVizhofok = new Highcharts.Chart({
      chart:{ renderTo : 'chart-Vizhofok' },
      title: { text: 'Víz hőmérséklet' },
      series: [{
        showInLegend: false,
        data: []
      }],
      plotOptions: {
        line: { animation: false,
        dataLabels: { enabled: true }
        },
        series: { color: '#059e8a' }
      },
      xAxis: { type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
      },
      yAxis: {
        title: { text: 'Hőmérséklet (Celsius)' }
      },
      credits: { enabled: false }
      });
    var lastValueVizhofok = null;
    var minVizhofok = null;
    var maxVizhofok = null;
    document.getElementById('reset-button-Vizhofok').addEventListener('click', function() {
      chartVizhofok.series[0].setData([], false);
      lastValueVizhofok = null;
      minVizhofok = null;
      maxVizhofok = null;
      document.getElementById('min-Vizhofok').textContent = '';
      document.getElementById('max-Vizhofok').textContent = '';
    });


    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);


          if (lastValueVizhofok === null) {
            minVizhofok = y;
            maxVizhofok = y;
          } 
          else {
            minVizhofok = Math.min(minVizhofok, y);
            maxVizhofok = Math.max(maxVizhofok, y);
          }
          if(chartVizhofok.series[0].data.length > 40) {
            chartVizhofok.series[0].addPoint([x, y], true, true, true);
          } 
          else {
            chartVizhofok.series[0].addPoint([x, y], true, false, true);
          }
        lastValueVizhofok = y;
        document.getElementById('min-Vizhofok').textContent = minVizhofok;
        document.getElementById('max-Vizhofok').textContent = maxVizhofok;
        }
      };
      xhttp.open('GET', '/vizhofok', true);
      xhttp.send();
    }, 1000 ) ;




















    var chartImap = new Highcharts.Chart({
      chart:{ renderTo:'chart-Imap' },
      title: { text: 'Turbó nyomás IMAP' },
      series: [{
        showInLegend: false,
        data: []
      }],
      plotOptions: {
        line: { animation: false,
        dataLabels: { enabled: true }
        }
      },
      xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
      },
      yAxis: {
        title: { text: 'Nyomás (bar)' }
      },
      credits: { enabled: false }
    });
    var lastValueImap = null;
    var minImap = null;
    var maxImap = null;
    document.getElementById('reset-button-Imap').addEventListener('click', function() {
      chartImap.series[0].setData([], false);
      lastValueImap = null;
      minImap = null;
      maxImap = null;
      document.getElementById('min-Imap').textContent = '';
      document.getElementById('max-Imap').textContent = '';
    });
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
        y = parseFloat(this.responseText);
        if (lastValueImap === null) {
          minImap = y;
          maxImap = y;
        } 
        else {
          minImap = Math.min(minImap, y);
          maxImap = Math.max(maxImap, y);
        }    
        if(chartImap.series[0].data.length > 40) {
          chartImap.series[0].addPoint([x, y], true, true, true);
        } 
        else {
          chartImap.series[0].addPoint([x, y], true, false, true);
        }
        lastValueImap = y;
        document.getElementById('min-Imap').textContent = minImap;
        document.getElementById('max-Imap').textContent = maxImap;
        }
      };
      xhttp.open('GET', '/imap', true);
      xhttp.send();
    }, 1000 ) ;

    var chartGyorsulas = new Highcharts.Chart({
      chart:{ renderTo:'chart-Gyorsulas' },
      title: { text: 'Gyorsulás' },
      series: [{
        showInLegend: false,
        data: []
      }],
      plotOptions: {
        line: { animation: false,
          dataLabels: { enabled: true }
        },
        series: { color: '#18009c' }
      },
      xAxis: {
        type: 'datetime',
        dateTimeLabelFormats: { second: '%H:%M:%S' }
      },
      yAxis: {
        title: { text: 'Gyorsulás (G)' }
      },
      credits: { enabled: false }
    });
    var lastValueGyorsulas = null;
    var minGyorsulas = null;
    var maxGyorsulas = null;
    document.getElementById('reset-button-Gyorsulas').addEventListener('click', function() {
      chartGyorsulas.series[0].setData([], false);
      lastValueGyorsulas = null;
      minGyorsulas = null;
      maxGyorsulas = null;
      document.getElementById('min-Gyorsulas').textContent = '';
      document.getElementById('max-Gyorsulas').textContent = '';
    });



    
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        var x = (new Date()).getTime(),
        y = parseFloat(this.responseText);
        if (lastValueGyorsulas === null) {
          minGyorsulas = y;
          maxGyorsulas = y;
        } 
        else {
          minGyorsulas = Math.min(minGyorsulas, y);
          maxGyorsulas = Math.max(maxGyorsulas, y);
        }
        if(chartGyorsulas.series[0].data.length > 40) {
          chartGyorsulas.series[0].addPoint([x, y], true, true, true);
        } 
        else {
          chartGyorsulas.series[0].addPoint([x, y], true, false, true);
        }
        lastValueGyorsulas = y;
        document.getElementById('min-Gyorsulas').textContent = minGyorsulas;
        document.getElementById('max-Gyorsulas').textContent = maxGyorsulas;
        }
      };
      xhttp.open('GET', '/gyorsulas', true);
      xhttp.send();
    }, 1000 ) ;
  </script>
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

//**************************************GRAFIKONOK************************************************************
// Oldal megnyitása
  server.on("/grafikon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", grafikon_html);
  });
// Vízhöfok küldése
  server.on("/vizhofok", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", VizhofoktoString().c_str());
  });
// Imap nyomás mérése
  server.on("/imap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ImaptoString().c_str());
  });
// Gyorsulás mérése
  server.on("/gyorsulas", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", GyorsulastoString().c_str());
  });
  // NotFound
  server.onNotFound(notFoundResponse);
// Szerverindítása
  server.begin();
//server.end();

}

// Loop függvény
void loop()
{
	if (espShouldReboot) {
		//Serial.println(F("Esp ujraindul ..."));
		delay(100);
		ESP.restart();
	}
  
  if(millis() >= time_now + period){
    time_now += period;
    vizhofok += 50.1;
    if(vizhofok >= 250.0) vizhofok=-25.0;
    imap_nyomas += 10.2;
    if(imap_nyomas >= 200.0) imap_nyomas=0.0;
    gyorsulas += 30.3;
    if(gyorsulas >= 150.0) gyorsulas=-10.0;
  }
}