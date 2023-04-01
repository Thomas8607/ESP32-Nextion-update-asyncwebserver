#include <ESPAsyncWebServer.h>


const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 100);


// Create Web Server
AsyncWebServer server(80);

bool check_status;
uint32_t fsize;


void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

const char *index_html PROGMEM = R"====(
<!DOCTYPE html>
    <html lang="en">
        <head>
            <meta http-equiv="content-type" content="text/html; charset=utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <title>Index page</title>
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
                        document.getElementById("button").disabled = false;
                        sendInfo(file.size);
                        var xhttp = new XMLHttpRequest();
                        xhttp.onreadystatechange = function(){
                        if(xhttp.readyState == 4 && xhttp.status == 200) {
                           alert('Ok status');
                        } 
                        else {
                          alert('Invalid status');
                        }
                     };
                    } else {
                        alert("Invalid file type. Only .tft files are allowed.");
                    }
                } else {
                    alert("Choose firmware file.");
                }
            }
            function sendInfo(size) {
                var xmlHttp = new XMLHttpRequest();
                xmlHttp.open("post", "/size");
                xmlHttp.send(size);
            }
            function sendData() {
              alert('Now this is a test')
            }
            </script>
        </head>
    <body>
        <input type="file" name="file" onchange="valCheck()">
        <input type="button" id="button" value="upload" onclick="sendData()" disabled>
	      <label id="completed"></label>
        Chunk size: <input type="text" name="partSize" value="1024" size="4">
    </body>
</html>
)====";


void setup()
{
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());

    // Index page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(index_html);
        request->send(response); 
    });

    // Receive Firmware file size
    server.on("/size", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            fsize = atoi((const char *)data);
            Serial.println("File size: " + String(fsize) + "bytes");
            check_status = true;
                                       
            if (check_status)
            {
                request->send(200, "text/plain", "FAIL CONNECTION");
                Serial.println("Check status Fail");
            }
            else {
                Serial.println("Check status Ok");
                request->send(200);
            }
        });
    server.onNotFound(notFound);
    server.begin();
}

void loop() {

}