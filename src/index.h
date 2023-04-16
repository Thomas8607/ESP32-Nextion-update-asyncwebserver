#ifndef INDEX_H
#define INDEX_H


const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <title>ESP32 Grafikonok</title>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <script src='https://code.highcharts.com/highcharts.js'></script>
    <script src='https://code.highcharts.com/modules/exporting.js'></script>
    <script src='https://code.highcharts.com/modules/export-data.js'></script>
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
    <script>
        // Hűtőfolyadék grafikon inicializálása
        var coolantChart = Highcharts.chart('coolantChartContainer', {
            turboThreshold: 0,
            title: {
                text: null, // Cím kikapcsolása
                enabled: false // Cím kikapcsolása
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: {
                    enabled: false // Pontok jelölésének kikapcsolása
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
            credits: { enabled: false },
            exporting: { // Exportálás beállításai
                buttons: {
                    contextButton: {
                        menuItems: [{
                        text: 'Export to PDF', // PDF export menüpont
                            onclick: function () {
                                this.exportChart({
                                type: 'application/pdf'
                                });
                            }
                        }]
                    }
                }
            }
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
        var imapChart = Highcharts.chart('imapChartContainer', {
            title: {
                text: null, // Cím kikapcsolása
                enabled: false // Cím kikapcsolása
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: {
                    enabled: false // Pontok jelölésének kikapcsolása
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
            credits: { enabled: false },
            exporting: { // Exportálás beállításai
                buttons: {
                    contextButton: {
                        menuItems: [{
                        text: 'Export to PDF', // PDF export menüpont
                            onclick: function () {
                                this.exportChart({
                                type: 'application/pdf'
                                });
                            }
                        }]
                    }
                }
            }
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


        // Websocket üzenetek fogadása és feldolgozása
        var ws = new WebSocket("ws://" + window.location.hostname + "/ws");
        ws.onmessage = function(event) {
            var data = JSON.parse(event.data);
            console.log('Adatok érkeztek:', data);
            var x = (new Date()).getTime();
            var y = data.coolanttemp;
            var y2 = data.imap;
            if (lastValueVizhofok === null) {
                minVizhofok = y;
                maxVizhofok = y;
            } 
            else {
                minVizhofok = Math.min(minVizhofok, y);
                maxVizhofok = Math.max(maxVizhofok, y);
            }
            if (coolantChart.series[0].data.length < 80) {
                coolantChart.series[0].addPoint([x, y], true, false, false);
            }
            else {
                coolantChart.series[0].addPoint([x, y], true, true, false);
            }
            lastValueVizhofok = y;
            document.getElementById('mincooltemp').textContent = minVizhofok.toFixed(1);
            document.getElementById('maxcooltemp').textContent = maxVizhofok.toFixed(1);
            document.getElementById("cooltemp").innerHTML = y.toFixed(1);

            if (lastValueImap === null) {
                minImap = y2;
                maxImap = y2;
            } 
            else {
                minImap = Math.min(minImap, y2);
                maxImap = Math.max(maxImap, y2);
            }
            if (imapChart.series[0].data.length < 80) {
                imapChart.series[0].addPoint([x, y2], true, false, false);
            } else {
                imapChart.series[0].addPoint([x, y2], true, true, false);
            }
            lastValueImap = y2;
            document.getElementById('minimap').textContent = minImap.toFixed(1);
            document.getElementById('maximap').textContent = maxImap.toFixed(1);
            document.getElementById("imap").innerHTML = y2.toFixed(1);
        };
    </script>
</body>
</html>
)=====";

#endif