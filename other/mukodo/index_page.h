#ifndef INDEX_PAGE_H
#define INDEX_PAGE_H

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
        text-align: center; 
      }
      .btn1 {
        background: LightGreen;
        width: 100%;
        height: 100px;
        border-radius: 20px;
        font-size:20px;
        text-align: center;
        display:block;
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
    <input type="button" class="btn1" value="ESP32 processzor firmware frissítés" onclick="window.location.href='/esp';" /><br><br><br><br>
    <input type="button" class="btn2" value="Nextion kijelző firmware frissítés" onclick="window.location.href='/nextion';" /><br><br><br><br>
    <input type="button" class="btn3" value="Grafikon megjelenítés" onclick="window.location.href='/grafikon';" />
    </form>
</body>
</html>
)====";

#endif

/*
    <button class="btn3" onclick="window.location.href='/grafikon';">
      <span>Grafikon megjelenítés</span></button>
*/




