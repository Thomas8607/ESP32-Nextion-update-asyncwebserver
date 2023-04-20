#ifndef INDEX_H
#define INDEX_H

const char index_html[] PROGMEM = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>ESP32 Grafikonok</title>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <script src='/highcharts.js'></script>
    <script src='/exporting.js'></script>
    <script src='/offline-exporting.js'></script>
    <style>
      html {
            text-align: center;
            font-family: Arial, Helvetica, sans-serif;
        }
        #title {
            margin: 20px auto;
            text-decoration: underline;
        }
        #data {
            font-size: 15px;
            text-align: left;
            margin: 5px auto;
        }
        #button {
            font-size: 15px;
            text-align: left;
            margin: 5px auto;
        }
    </style>
</head>
<body>
    <h3 id="title">Adat megjelenítés</h3>
    <br>
    <p id="data">
    <span>Vízhőmérséklet: </span><span id="cooltemp">0</span><span> °C</span> &emsp;
    <span>Min. érték: </span><span id="mincooltemp">0</span><span> °C</span> &emsp;
    <span>Max. érték: </span><span id="maxcooltemp">0</span><span> °C</span> &emsp;
    <input type="button" id="coolantresetbutton" value="Törlés"> &emsp;&emsp;
    <div id='coolantChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Imap nyomás: </span><span id="imap">0</span><span> bar</span> &emsp;
    <span>Min. érték: </span><span id="minimap">0</span><span> bar</span> &emsp;
    <span>Max. érték: </span><span id="maximap">0</span><span> bar</span> &emsp;
    <input type="button" id="imapresetbutton" value="Törlés"> &emsp;&emsp;
    <div id='imapChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Emap nyomás: </span><span id="emap">0</span><span> bar</span> &emsp;
    <span>Min. érték: </span><span id="minemap">0</span><span> bar</span> &emsp;
    <span>Max. érték: </span><span id="maxemap">0</span><span> bar</span> &emsp;
    <input type="button" id="emapresetbutton" value="Törlés"> &emsp;&emsp;
    <div id='emapChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <script>
        // Hűtőfolyadék grafikon inicializálása
        var coolantChart = new Highcharts.chart({
            chart: { 
                renderTo : 'coolantChartContainer'
            },
            turboThreshold: 0,
            title: {
                text: 'Hűtőfolyadék hőmérséklet',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: {
                    enabled: false
                }
            }],
            plotOptions: {
                line: { animation: false,
                dataLabels: { enabled: false }
                },
                series: { color: '#059e8a' }
            },
            xAxis: { type: 'datetime',
                dateTimeLabelFormats: { second: '%H:%M:%S' }
            },
            yAxis: {
                title: { text: 'hőmérséklet (ºC)' }
            },
            credits: { enabled: false }
        });
        var lastValueVizhofok = null;
        var minVizhofok = null;
        var maxVizhofok = null;
        document.getElementById('coolantresetbutton').addEventListener('click', function() {
            coolantChart.series[0].setData([], false);
            lastValueVizhofok = null;
            minVizhofok = null;
            maxVizhofok = null;
            document.getElementById('mincooltemp').textContent = '';
            document.getElementById('maxcooltemp').textContent = '';
        });
        // IMAP grafikon inicializálása
        var imapChart = new Highcharts.chart({
            chart: {
                renderTo: 'imapChartContainer'
            },
            title: {
                text: 'Imap nyomás',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: {
                    enabled: false
                    }
            }],
            plotOptions: {
                line: { animation: false,
                dataLabels: { enabled: false }
                },
                series: { color: '#059e8a' }
            },
            xAxis: { type: 'datetime',
                dateTimeLabelFormats: { second: '%H:%M:%S' }
            },
            yAxis: {
                title: { text: 'nyomás (bar)' }
            },
            credits: { enabled: false }
        });
        var lastValueImap = null;
        var minImap = null;
        var maxImap = null;
        document.getElementById('imapresetbutton').addEventListener('click', function() {
            imapChart.series[0].setData([], false);
            lastValueImap = null;
            minImap = null;
            maxImap = null;
            document.getElementById('minimap').textContent = '';
            document.getElementById('maximap').textContent = '';
        });
        // EMAP grafikon inicializálása
        var emapChart = new Highcharts.chart({
            chart: {
                renderTo: 'emapChartContainer' 
            },
            title: {
                text: 'Emap nyomás',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: {
                    enabled: false
                    }
            }],
            plotOptions: {
                line: { animation: false,
                dataLabels: { enabled: false }
                },
                series: { color: '#059e8a' }
            },
            xAxis: { type: 'datetime',
                dateTimeLabelFormats: { second: '%H:%M:%S' }
            },
            yAxis: {
                title: { text: 'nyomás (bar)' }
            },
            credits: { enabled: false }
        });
        var lastValueEmap = null;
        var minEmap = null;
        var maxEmap = null;
        document.getElementById('emapresetbutton').addEventListener('click', function() {
            emapChart.series[0].setData([], false);
            lastValueEmap = null;
            minEmap = null;
            maxEmap = null;
            document.getElementById('minemap').textContent = '';
            document.getElementById('maxemap').textContent = '';
        });

        // Websocket üzenetek fogadása és feldolgozása
        var ws = new WebSocket("ws://" + window.location.hostname + "/ws");
        ws.onmessage = function(event) {
            var data = JSON.parse(event.data);
            console.log('Adatok érkeztek:', data);
            var x_ido = (new Date()).getTime();
            var y_coolant = data.coolanttemp;
            var y_imap = data.imap;
            var y_emap = data.emap;

            // CoolantTemperature
            if (lastValueVizhofok === null) {
                minVizhofok = y_coolant;
                maxVizhofok = y_coolant;
            } 
            else {
                minVizhofok = Math.min(minVizhofok, y_coolant);
                maxVizhofok = Math.max(maxVizhofok, y_coolant);
            }
            if (coolantChart.series[0].data.length < 80) {
                coolantChart.series[0].addPoint([x_ido, y_coolant], true, false, false);
            }
            else {
                coolantChart.series[0].addPoint([x_ido, y_coolant], true, true, false);
            }
            lastValueVizhofok = y_coolant;
            document.getElementById('mincooltemp').textContent = minVizhofok.toFixed(1);
            document.getElementById('maxcooltemp').textContent = maxVizhofok.toFixed(1);
            document.getElementById("cooltemp").innerHTML = y_coolant.toFixed(1);
            // Imap
            if (lastValueImap === null) {
                minImap = y_imap;
                maxImap = y_imap;
            } 
            else {
                minImap = Math.min(minImap, y_imap);
                maxImap = Math.max(maxImap, y_imap);
            }
            if (imapChart.series[0].data.length < 80) {
                imapChart.series[0].addPoint([x_ido, y_imap], true, false, false);
            } else {
                imapChart.series[0].addPoint([x_ido, y_imap], true, true, false);
            }
            lastValueImap = y_imap;
            document.getElementById('minimap').textContent = minImap.toFixed(1);
            document.getElementById('maximap').textContent = maxImap.toFixed(1);
            document.getElementById("imap").innerHTML = y_imap.toFixed(1);
            // Emap
            if (lastValueEmap === null) {
                minEmap = y_emap;
                maxEmap = y_emap;
            } 
            else {
                minEmap = Math.min(minEmap, y_emap);
                maxEmap = Math.max(maxEmap, y_emap);
            }
            if (emapChart.series[0].data.length < 80) {
                emapChart.series[0].addPoint([x_ido, y_emap], true, false, false);
            } else {
                emapChart.series[0].addPoint([x_ido, y_emap], true, true, false);
            }
            lastValueEmap = y_emap;
            document.getElementById('minemap').textContent = minEmap.toFixed(1);
            document.getElementById('maxemap').textContent = maxEmap.toFixed(1);
            document.getElementById("emap").innerHTML = y_emap.toFixed(1);
        };
    </script>
</body>
</html>
)====";
#endif