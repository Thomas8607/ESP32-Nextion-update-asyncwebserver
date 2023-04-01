#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Wifi adatok
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 100);

float t;
float h;
float p;


const char* grafikon_html = R"(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1" meta charset="UTF-8">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <script src="https://code.highcharts.com/modules/exporting.js"></script>
  <script src="https://code.highcharts.com/modules/export-data.js"></script>
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
  <div id="chart-Vizhofok" class="container"></div>
  <div>Minimum érték: <span id="min-Vizhofok"></span></div>
  <div>Maximum érték: <span id="max-Vizhofok"></span>
  <button id="reset-button-Vizhofok">Törlés</button></div>
  <div id="chart-Imap" class="container"></div>
  <div>Minimum érték: <span id="min-Imap"></span></div>
  <div>Maximum érték: <span id="max-Imap"></span>
  <button id="reset-button-Imap">Törlés</button></div>
  <div id="chart-Gyorsulas" class="container"></div>
  <div>Minimum érték: <span id="min-Gyorsulas"></span></div>
  <div>Maximum érték: <span id="max-Gyorsulas"></span>
  <button id="reset-button-Gyorsulas">Törlés</button></div>
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
      } else {
        minVizhofok = Math.min(minVizhofok, y);
        maxVizhofok = Math.max(maxVizhofok, y);
      }
      if(chartVizhofok.series[0].data.length > 40) {
        chartVizhofok.series[0].addPoint([x, y], true, true, true);
      } else {
        chartVizhofok.series[0].addPoint([x, y], true, false, true);
      }
      lastValueVizhofok = y;
      document.getElementById('min-Vizhofok').textContent = minVizhofok;
      document.getElementById('max-Vizhofok').textContent = maxVizhofok;
    }
  };
  xhttp.open("GET", "/vizhofok", true);
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
      } else {
        minImap = Math.min(minImap, y);
        maxImap = Math.max(maxImap, y);
      }    
      if(chartImap.series[0].data.length > 40) {
        chartImap.series[0].addPoint([x, y], true, true, true);
      } else {
        chartImap.series[0].addPoint([x, y], true, false, true);
      }
      lastValueImap = y;
      document.getElementById('min-Imap').textContent = minImap;
      document.getElementById('max-Imap').textContent = maxImap;
    }
  };
  xhttp.open("GET", "/imap", true);
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
      } else {
        minGyorsulas = Math.min(minGyorsulas, y);
        maxGyorsulas = Math.max(maxGyorsulas, y);
      }
      if(chartGyorsulas.series[0].data.length > 40) {
        chartGyorsulas.series[0].addPoint([x, y], true, true, true);
      } else {
        chartGyorsulas.series[0].addPoint([x, y], true, false, true);
      }
      lastValueGyorsulas = y;
      document.getElementById('min-Gyorsulas').textContent = minGyorsulas;
      document.getElementById('max-Gyorsulas').textContent = maxGyorsulas;
    }
  };
  xhttp.open("GET", "/gyorsulas", true);
  xhttp.send();
}, 1000 ) ;
</script>
</html>
)";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String VizhofokMeres() {
  if (isnan(t)) {    
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String ImapMeres() {
  if (isnan(h)) {
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String GyorsulasMeres() {
  if (isnan(p)) {
    Serial.println("Rossz ertek a szenzortol!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}

void setup(){
  // Serial port inicializálása
  Serial.begin(115200);
  //Kapcsolódunk az AP-hez
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
  //Kiírjuk az IP-címet
  Serial.println("AP IP címe: " + WiFi.softAPIP().toString());
  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", grafikon_html);
  });
  server.on("/vizhofok", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", VizhofokMeres().c_str());
  });
  server.on("/imap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ImapMeres().c_str());
  });
  server.on("/gyorsulas", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", GyorsulasMeres().c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){
  t += 50.1;
  if(t>=250.0) t=-25.0;
  h += 10.2;
  if(h>=200.0) h=0.0;
  p += 30.3;
  if(p>=150.0) p=-10.0;
  delay(200);
}