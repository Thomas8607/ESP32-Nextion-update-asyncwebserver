#include <ESPAsyncWebServer.h>
#include <NextionUploadWIFI.h>


const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 10, 100);
IPAddress gateway(192, 168, 10, 100);


// Create Web Server
AsyncWebServer server(80);
// Create Nextion WiFi Uploader object
NextionUploadWIFI nextion(115200, GPIO_NUM_41, GPIO_NUM_42);


uint32_t filesize;
bool upload_status;
String check_reason = "";
String error_reason = "";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}
// Nextion update index page
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
                    if (xmlHttp.readyState == 4 && (xmlHttp.status == 302 || xmlHttp.status == 404)) {
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
        <input type="file" name="file" onchange="valCheck()"><br><br>
        <input type="button" id="button" value="upload" onclick="sendData()" disabled><br><br>
        <span id="uploading" style="display:none;">Upload in process...</span><br>
	      <label id="completed"></label><br>
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
    <title>Nextion display updater</title>
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
// Nextion update success
const char *nextion_update_success_html PROGMEM = R"====(
<!DOCTYPE html>
  <html lang="en">
    <head>
      <meta http-equiv="content-type" content="text/html; charset=utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>Nextion display updater</title>
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
        <h1><strong>Successfull update!</strong></h1>
        <br>
        <br>
        <h3>Display will restart!</h3>
        <input type="button" class="btn" value="Home page" onclick="window.location.href='/'">
      </form>
    </body>
  </html>
)====";
//***********************************************************************SETUP************************************************************************************************
void setup() {
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_IP, gateway, IPAddress(255, 255, 255, 0));
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());

// Index page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(index_html);
        request->send(response); 
    });
// Fail page
    server.on("/nextion_fail", HTTP_GET, [](AsyncWebServerRequest *request) {
		String view_html;
		view_html += nextion_update_failed_header_html;
		view_html += "<label><h3>Error reason: " + error_reason + "</h3></label>";
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

    // Receive Firmware cunks and flash Nextion display
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            request->send(200);
            if(!nextion.uploadTftFile(data, len)) {
                request->send(200);
            }
            else {
                error_reason = "Connection lost!";
                request->redirect("/nextion_fail");
            }
    });
    server.onNotFound(notFound);
    server.begin();
}
//************************************************************************************LOOP***************************************************************************************************
void loop() {

}


/*
Egy tizedes pontos
cmp.innerText = Math.round(offset * 100 / file.size) + "%";

végén

*/