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
    <span>Fordulatszám: </span><span id="rpm">0</span><span> 1/f</span> &emsp;
    <span>Min. érték: </span><span id="minrpm">0</span><span> 1/f</span> &emsp;
    <span>Max. érték: </span><span id="maxrpm">0</span><span> 1/f</span> &emsp;
    <input type="button" id="rpmresetbutton" value="Törlés">
    <div id='rpmChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Vízhőmérséklet: </span><span id="cooltemp">0</span><span> °C</span> &emsp;
    <span>Min. érték: </span><span id="mincooltemp">0</span><span> °C</span> &emsp;
    <span>Max. érték: </span><span id="maxcooltemp">0</span><span> °C</span> &emsp;
    <input type="button" id="coolantresetbutton" value="Törlés">
    <div id='coolantChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Imap nyomás: </span><span id="imap">0</span><span> bar</span> &emsp;
    <span>Min. érték: </span><span id="minimap">0</span><span> bar</span> &emsp;
    <span>Max. érték: </span><span id="maximap">0</span><span> bar</span> &emsp;
    <input type="button" id="imapresetbutton" value="Törlés">
    <div id='imapChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Emap nyomás: </span><span id="emap">0</span><span> bar</span> &emsp;
    <span>Min. érték: </span><span id="minemap">0</span><span> bar</span> &emsp;
    <span>Max. érték: </span><span id="maxemap">0</span><span> bar</span> &emsp;
    <input type="button" id="emapresetbutton" value="Törlés">
    <div id='emapChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Beszívott levegő hőmérséklet: </span><span id="intake">0</span><span> °C</span> &emsp;
    <span>Min. érték: </span><span id="minintake">0</span><span> °C</span> &emsp;
    <span>Max. érték: </span><span id="maxintake">0</span><span> °C</span> &emsp;
    <input type="button" id="intakeresetbutton" value="Törlés">
    <div id='intakeChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <br>
    <hr>
    <br>
    <p id="data">
    <span>Gyorsulás: </span><span id="accel">0</span><span> G</span> &emsp;
    <span>Min. érték: </span><span id="minaccel">0</span><span> G</span> &emsp;
    <span>Max. érték: </span><span id="maxaccel">0</span><span> G</span> &emsp;
    <input type="button" id="accelresetbutton" value="Törlés">
    <div id='accelChartContainer' class='container' style="position: relative; width:90vw; margin:auto"></div>
    </p>
    <script>
// Fordulatszám grafikon inicializálása
        var rpmChart = new Highcharts.chart({
            chart: {  renderTo : 'rpmChartContainer' },
            turboThreshold: 0,
            title: {
                text: 'Fordulatszám',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
                title: { text: 'fordulatszám (1/f)' }
            },
            credits: { enabled: false },
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
                    }
                }
            }
        });
        var lastValueRpm = null;
        var minRpm = null;
        var maxRpm = null;
        document.getElementById('rpmresetbutton').addEventListener('click', function() {
            rpmChart.series[0].setData([], false);
            lastValueRpm = null;
            minRpm = null;
            maxRpm = null;
            document.getElementById('minrpm').textContent = '';
            document.getElementById('maxrpm').textContent = '';
        });
// Hűtőfolyadék grafikon inicializálása
        var coolantChart = new Highcharts.chart({
            chart: {  renderTo : 'coolantChartContainer' },
            turboThreshold: 0,
            title: {
                text: 'Hűtőfolyadék hőmérséklet',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
                    }
                }
            }
        });
        var lastValueCoolant = null;
        var minCoolantTemp = null;
        var maxCoolantTemp = null;
        document.getElementById('coolantresetbutton').addEventListener('click', function() {
            coolantChart.series[0].setData([], false);
            lastValueCoolant = null;
            minCoolantTemp = null;
            maxCoolantTemp = null;
            document.getElementById('mincooltemp').textContent = '';
            document.getElementById('maxcooltemp').textContent = '';
        });
// IMAP grafikon inicializálása
        var imapChart = new Highcharts.chart({
            chart: { renderTo: 'imapChartContainer' },
            title: {
                text: 'Imap nyomás',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
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
// EMAP grafikon inicializálása
        var emapChart = new Highcharts.chart({
            chart: { renderTo: 'emapChartContainer' },
            title: {
                text: 'Emap nyomás',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
                    }
                }
            }
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
// IntakeAirTemp grafikon inicializálása
        var IntakeAirChart = new Highcharts.chart({
            chart: { renderTo: 'intakeChartContainer' },
            title: {
                text: 'Beszívott levegő hőmérséklet',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
                    }
                }
            }
        });
        var lastValueIntakeAir = null;
        var minIntakeAir = null;
        var maxIntakeAir = null;
        document.getElementById('intakeresetbutton').addEventListener('click', function() {
            IntakeAirChart.series[0].setData([], false);
            lastValueIntakeAir = null;
            minIntakeAir = null;
            maxIntakeAir = null;
            document.getElementById('minintake').textContent = '';
            document.getElementById('maxintake').textContent = '';
        });
// Acceleration grafikon inicializálása
        var AccelerationChart = new Highcharts.chart({
            chart: { renderTo: 'accelChartContainer' },
            title: {
                text: 'Gyorsulás',
                align: 'left'
            },
            series: [{
                showInLegend: false,
                data: [],
                marker: { enabled: false }
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
                title: { text: 'gyorsulás (G)' }
            },
            credits: { enabled: false },
            accessibility: { enabled: false },
            exporting: {
                buttons: {
                    contextButton: {
                        menuItems: ["viewFullscreen", "downloadJPEG"]
                    }
                }
            }
        });
        var lastValueAcceleration = null;
        var minAcceleration = null;
        var maxAcceleration = null;
        document.getElementById('accelresetbutton').addEventListener('click', function() {
            AccelerationChart.series[0].setData([], false);
            lastValueAcceleration = null;
            minAcceleration = null;
            maxAcceleration = null;
            document.getElementById('minaccel').textContent = '';
            document.getElementById('maxaccel').textContent = '';
        });
// Websocket
        var webSocket = new WebSocket("ws://" + window.location.hostname + "/ws");
        webSocket.binaryType = 'arraybuffer';
        webSocket.onmessage = function(event) {
            var dataView = new DataView(event.data);
            var x_time = dataView.getUint32(0, true);
            var rpm = dataView.getUint16(4, true);
            var coolantTemp = dataView.getInt16(6, true)/10;
            var imapPressure = dataView.getInt16(8, true)/100;
            var emapPressure = dataView.getInt16(10, true)/100;
            var intakeairTemp = dataView.getInt16(12, true)/10;
            var acceleration = dataView.getInt16(14, true)/100;
            //console.log("X_Time: " + x_time + ", Rpm: " + rpm + ", CoolantTemp: " + coolantTemp + ", Imap: " + imapPressure + ", Emap: " + emapPressure + ", IntakeairTemp: " + intakeairTemp + ", Acceleration: " + acceleration);
// Rpm
            if (lastValueRpm === null) {
                minRpm = rpm;
                maxRpm = rpm;
            } 
            else {
                minRpm = Math.min(minRpm, rpm);
                maxRpm = Math.max(maxRpm, rpm);
            }
            if (rpmChart.series[0].data.length < 150) {
                rpmChart.series[0].addPoint([x_time, rpm], true, false, false);
            }
            else {
                rpmChart.series[0].addPoint([x_time, rpm], true, true, false);
            }
            lastValueRpm = rpm;
            document.getElementById('minrpm').textContent = minRpm;
            document.getElementById('maxrpm').textContent = maxRpm;
            document.getElementById("rpm").innerHTML = rpm;
// CoolantTemperature
            if (lastValueCoolant === null) {
                minCoolantTemp = coolantTemp;
                maxCoolantTemp = coolantTemp;
            } 
            else {
                minCoolantTemp = Math.min(minCoolantTemp, coolantTemp);
                maxCoolantTemp = Math.max(maxCoolantTemp, coolantTemp);
            }
            if (coolantChart.series[0].data.length < 150) {
                coolantChart.series[0].addPoint([x_time, coolantTemp], true, false, false);
            }
            else {
                coolantChart.series[0].addPoint([x_time, coolantTemp], true, true, false);
            }
            lastValueCoolant = coolantTemp;
            document.getElementById('mincooltemp').textContent = minCoolantTemp;
            document.getElementById('maxcooltemp').textContent = maxCoolantTemp;
            document.getElementById("cooltemp").innerHTML = coolantTemp;
// Imap
            if (lastValueImap === null) {
                minImap = imapPressure;
                maxImap = imapPressure;
            } 
            else {
                minImap = Math.min(minImap, imapPressure);
                maxImap = Math.max(maxImap, imapPressure);
            }
            if (imapChart.series[0].data.length < 150) {
                imapChart.series[0].addPoint([x_time, imapPressure], true, false, false);
            } else {
                imapChart.series[0].addPoint([x_time, imapPressure], true, true, false);
            }
            lastValueImap = imapPressure;
            document.getElementById('minimap').textContent = minImap;
            document.getElementById('maximap').textContent = maxImap;
            document.getElementById("imap").innerHTML = imapPressure;
// Emap
            if (lastValueEmap === null) {
                minEmap = emapPressure;
                maxEmap = emapPressure;
            } 
            else {
                minEmap = Math.min(minEmap, emapPressure);
                maxEmap = Math.max(maxEmap, emapPressure);
            }
            if (emapChart.series[0].data.length < 150) {
                emapChart.series[0].addPoint([x_time, emapPressure], true, false, false);
            } else {
                emapChart.series[0].addPoint([x_time, emapPressure], true, true, false);
            }
            lastValueEmap = emapPressure;
            document.getElementById('minemap').textContent = minEmap;
            document.getElementById('maxemap').textContent = maxEmap;
            document.getElementById("emap").innerHTML = emapPressure;
// IntakeAirTemp
            if (lastValueIntakeAir === null) {
                minIntakeAir = intakeairTemp;
                maxIntakeAir = intakeairTemp;
            } 
            else {
                minIntakeAir = Math.min(minIntakeAir, intakeairTemp);
                maxIntakeAir = Math.max(maxIntakeAir, intakeairTemp);
            }
            if (IntakeAirChart.series[0].data.length < 150) {
                IntakeAirChart.series[0].addPoint([x_time, intakeairTemp], true, false, false);
            } else {
                IntakeAirChart.series[0].addPoint([x_time, intakeairTemp], true, true, false);
            }
            lastValueIntakeAir = intakeairTemp;
            document.getElementById('minintake').textContent = minIntakeAir;
            document.getElementById('maxintake').textContent = maxIntakeAir;
            document.getElementById("intake").innerHTML = intakeairTemp;
// Acceleration
            if (lastValueAcceleration === null) {
                minAcceleration = acceleration;
                maxAcceleration = acceleration;
            } 
            else {
                minAcceleration = Math.min(minAcceleration, acceleration);
                maxAcceleration = Math.max(maxAcceleration, acceleration);
            }
            if (AccelerationChart.series[0].data.length < 150) {
                AccelerationChart.series[0].addPoint([x_time, acceleration], true, false, false);
            } else {
                AccelerationChart.series[0].addPoint([x_time, acceleration], true, true, false);
            }
            lastValueAcceleration = acceleration;
            document.getElementById('minaccel').textContent = minAcceleration;
            document.getElementById('maxaccel').textContent = maxAcceleration;
            document.getElementById("accel").innerHTML = acceleration;
        };        
        webSocket.onopen = function(event) {
            console.log("WebSocket opened");
        };
        webSocket.onclose = function(event) {
            console.log("WebSocket closed");
        };
        webSocket.onerror = function(event) {
            console.log("WebSocket error: " + event);
        };
    </script>
</body>
</html>
)====";
#endif