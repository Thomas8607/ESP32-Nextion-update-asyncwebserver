#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>


// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 100);
String FW_VERSION = "1.0";
bool espShouldReboot;
AsyncWebServer server(80);

// NexUploadWIFI nex_uploader(115200);
int fileSize  = 0;
bool result   = true;
bool nextionShouldReboot;
uint32_t time_now; 
uint32_t period = 100;
float vizhofok;
float imap_nyomas;
float gyorsulas;

String VizhofoktoString();
String ImaptoString();
String GyorsulastoString();
void notFoundResponse(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  Serial.println("Beleptunk ide");
  Serial.println(len);
  
}
//*********************************************************************KIVÁLASZTÓ MENÜ***********************
const char* index_html PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'/>
    <meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'>
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
)";
//*************************************************************************ESP32 FRISSITO OLDALAK*************************************************************************************
// ESP32 frissítő oldal első része
const char* esp_header_html PROGMEM = R"(
<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv='Content-Type' content='text/html; charset=utf-8'/>
    <meta name='viewport' content='width=device-width, initial-scale=1, minimum-scale=1.0, shrink-to-fit=no'>
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
  </head>
  <body>
    <form method='POST' action='/esp_update' enctype='multipart/form-data'>
      <label><b><h1>ESP32 processzor frissítő</h1></b></label>
)";
// ESP32 frissítő oldal Verziószám utáni része
const char* esp_update_html PROGMEM = R"(
      <input accept='.bin' type='file' name='update' required>
      <h5>Csak .bin fájl engedélyezett</h5>
      <input type='submit' value='Frissítés kezdés'>
    </form>
  </body>
</html>
)";
// ESP32 frissítés sikeres oldal
const char* esp_update_success_html PROGMEM = R"(
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
        <h1><strong>Frissítés sikeres</strong></h1>
        <h3>Az ESP32 újraindul!</h3>
        <input type="button" class="btn" value="Vissza a kezdőoldalra" onclick="window.location.href='/esp'"/>
      </form>
    </body>
  </html>
)";
// ESP32 frissítés sikertelen oldal
const char* esp_update_failed_html PROGMEM = R"(
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
        <h1><strong>Frissítés sikertelen</strong></h1>
        <input type="button" class="btn" value="Vissza a kezdőoldalra" onclick="window.location.href='/esp'"/>
      </form>
    </body>
  </html>
)";
//*******************************************************NEXTION FRISSÍTŐ OLDALAK******************************************************************************************





//*************************************************GRAFIKONOK*********************************************
const char* grafikon_html = R"(
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
)";
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
			Serial.printf("Start update: %s\n", filename.c_str());
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
				Serial.printf("Update was successfull: %uB\n", index + len);
			}
			else {
				Update.printError(Serial);
			}
		} 
	});
//***********************************************NEXTION UPDATE************************************************************************************************* 


















// NotFound
  server.onNotFound(notFoundResponse);
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
// Szerverindítása
server.begin();
//server.end();

}
// Vízhőfok string átalakítás
String VizhofoktoString() {
  if (isnan(vizhofok)) {    
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(vizhofok);
    return String(vizhofok);
  }
}
// Imap string átalakítás
String ImaptoString() {
  if (isnan(imap_nyomas)) {
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(imap_nyomas);
    return String(imap_nyomas);
  }
}
// Gyorsulás string átalakítás
String GyorsulastoString() {
  if (isnan(gyorsulas)) {
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(gyorsulas);
    return String(gyorsulas);
  }
}
// Loop függvény
void loop()
{
	if (espShouldReboot)
	{
		Serial.println(F("Esp ujraindul ..."));
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