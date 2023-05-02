#ifndef NEXTION_PAGES_H
#define NEXTION_PAGES_H

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



#endif