#include <NexUploadWIFI.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


const char *ssid = "ESP_proba";
const char *password = "123456789";
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 100);

// Create Nextion WiFi Uploader object
NexUploadWIFI nex_uploader(115200);

// Create Web Server
AsyncWebServer server(80);

bool check_status;
bool upload_status;
uint32_t fsize;
uint32_t time_now;
uint32_t period = 100;
String error_reason = "Connection lost";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}





const char *nextion_html PROGMEM = R"====(
<!DOCTYPE html>
    <html lang="en">
        <head>
            <meta http-equiv="content-type" content="text/html; charset=utf-8">
            <meta name="viewport" content="width=device-width, initial-scale=1">
            <title>Nextion kijelző frissítő</title>
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

// Nextion frissítés sikeres oldal
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
        <h1><strong>Frissítés sikeres</strong></h1>
        <input type="button" class="btn" value="Vissza a kezdőoldalra" onclick="window.location.href='/'">
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

    // Nextion index page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print(nextion_html);
        request->send(response); 
    });
/*
    // Fail page
    server.on("/nextion_fail", HTTP_GET, [](AsyncWebServerRequest *request)
              {
		String view_html;
		view_html += nextion_update_failed_header_html;
		view_html += "<label><h3>Error reason: " + error_reason + "</h3></label>";
		view_html += nextion_update_failed_footer_html;
    Serial.println("We are here");
    //request->send(200, "text/plain", "FAIL CONNECTION");
		request->send(302, "text/html", view_html); 
    });
*/

    // Receive Firmware file size
    server.on("/size", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            fsize = atoi((const char *)data);
            check_status = nex_uploader.check(fsize); // If check status is true, then we have any check errors(baudrate, or connection error). Open an error html page and don't start the uploading
                                                      // If check status is false, we will start the file uploading
            Serial.println("File size: " + String(fsize) + "bytes");
            if (check_status)
            {
                request->send(200, "text/plain", "FAIL CONNECTION");
                //request->redirect("/nextion_fail");
                Serial.println("Check status Fail");
            }
            else {
                Serial.println("Check status Ok");
                request->send(200);
            }
        });





/*
    // Receive Firmware cunks and flash Nextion
    server.on(
        "/u", HTTP_POST, [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            upload_status = nex_uploader.uploadTftFile(data, len); // If upload_status is false, then we have uploading error(s). Open another error html page, and close the file uploading.
                                                                   // If upload_status is true, the uploading is done. Open the successfull html page
            Serial.println("Upload status: " + String(upload_status));
            request->send(200);
        });
*/

    server.onNotFound(notFound);
    server.begin();
}

void loop() {

}

/*
javascript from html
            function sendDataHandler(event) {
                if (event.target.error == null) {
                    cmp.innerText = (offset * 100 / file.size).toFixed(1) + "%";
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
                            cmp.innerText = "All data was sent";
                        }
                    }
                };
                xmlHttp.open("post", "/u");
                xmlHttp.send(event.target.result);
            }
            function sendData() {
                document.getElementById("button").disabled = true;
                var reader = new FileReader();
                var blob = file.slice(offset, partSize + offset);
                reader.onload = sendDataHandler;
                reader.readAsArrayBuffer(blob);
            }


    */