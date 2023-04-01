
#include <ESPAsyncWebServer.h>
AsyncWebServer server(80);
const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 100);



const char* upload_path = "/upload";
const char* fail_html = "<html><body><h1>Invalid file extension or size!</h1></body></html>";
bool check_status;
bool upload_status;
uint32_t fsize;
uint32_t time_now;
uint32_t period = 100;
String error_reason = "Connection lost";

const char* index_html PROGMEM = R"====(
  <!DOCTYPE html>
<html>
<head>
    <title>Fájl méretének elküldése az ESP32-nek</title>
</head>
<body>
    <h1>Fájl méretének elküldése az ESP32-nek</h1>
    <form>
        <label for="file">Válassz fájlt:</label>
        <input type="file" id="file" name="file" onchange="valCheck()">
    </form>
    <script>
        function valCheck() {
            var file = document.getElementById("file").files[0];
            if (file) {
                sendInfo(file.size);
            }
        }
        function sendInfo(size) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    console.log(this.responseText);
                }
            };
            xhttp.open("POST", "/getSize", true);
            xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xhttp.send("size=" + size);
        }
    </script>
</body>
</html>

)====";



void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
  Serial.println("AP IP address: " + WiFi.softAPIP().toString());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });

    server.on("/getSize", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("size")) {
            String size = request->getParam("size")->value();
            Serial.print("Received size: ");
            Serial.println(size);
            request->send(200, "text/plain", "Size is OK");
        } else {
            request->send(400, "text/plain", "Missing size parameter");
        }
    });

  server.begin();
  
}

void loop() {

}
