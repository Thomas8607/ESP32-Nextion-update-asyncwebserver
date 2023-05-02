#ifndef ESP_PAGES_H
#define ESP_PAGES_H

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

#endif