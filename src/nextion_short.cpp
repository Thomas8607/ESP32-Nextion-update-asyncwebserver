#include <ESPAsyncWebServer.h>
#include "NexUploadWIFI.h"


const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);


// Create Web Server
AsyncWebServer server(80);
// Create Nextion WiFi Uploader object
NexUploadWIFI nextion(115200);


bool check_status;
uint32_t filesize;
String upload_reason = "";
String check_reason = "";
String error_reason = "";

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
                        alert("Invalid file type. Only .tft files are allowed.");
                    }
                } else {
                    alert("Choose firmware file.");
                }
            }
            function sendInfo(xmlHttp, size) {
                xmlHttp.open("post", "/size");
                xmlHttp.send(size);
            }
            function sendData() {
              alert("This is a test for upload button")
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

// Nextion update error header
const char *nextion_update_failed_header_html PROGMEM = R"====(
<!DOCTYPE html>
 <html lang="en">
    <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Nextion display update</title>
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
)====";

const char *nextion_update_failed_footer_html PROGMEM = R"====(
        <input type="button" class="btn" value="Home Page" onclick="window.location.href='/'">
      </form>
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
// Fail page
    server.on("/nextion_fail", HTTP_GET, [](AsyncWebServerRequest *request)
              {
		String view_html;
		view_html += nextion_update_failed_header_html;
		view_html += "<label><h3>Error reason: " + error_reason + "</h3></label>";
		view_html += nextion_update_failed_footer_html;
        request->send(302, "text/html", view_html);
    });

// Receive Firmware file size
    server.on("/size", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            filesize = atoi((const char *)data);
            check_reason = nextion.check(filesize);
            if (!check_reason.equals("0")) {
                error_reason = check_reason;
                Serial.println("Check status: " + error_reason);
                request->redirect("/nextion_fail");
            }
            else {
                Serial.println("Check status Ok");
                error_reason = "";
                Serial.println("File size: " + String(filesize) + "bytes");
                request->send(200);
            }
        });
    server.onNotFound(notFound);
    server.begin();
}

void loop() {

}