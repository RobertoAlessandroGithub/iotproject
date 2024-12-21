#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define DHTPIN D6       // Pin data DHT
#define DHTTYPE DHT22   // Tipe sensor DHT



DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

const char* soft_ap_name = "Kelembapan_Udara"; 
const char* ssid_password = "akulembab";     

const char* wifi_ssid = "Febrian"; //
const char* wifi_password = "nevermore";

void handleData() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Jika pembacaan gagal, kirimkan nilai error
    if (isnan(temperature) || isnan(humidity)) {
        server.send(500, "application/json", "{\"temperature\":null,\"humidity\":null}");
        return;
    }

    // Kirim data JSON
    String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    server.send(200, "application/json", json);
}

void handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Monitoring Temperatur dan Kelembapan</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; text-align: center; }";
    html += "h1 { color: #4CAF50; }";
    html += ".container { max-width: 800px; margin: 0 auto; padding: 20px; background-color: white; box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1); border-radius: 8px; }";
    html += "#chartContainer { margin-top: 30px; }";
    html += ".data-display { margin-top: 20px; font-size: 18px; font-weight: bold; }";
    html += ".data-display span { margin-right: 20px; }";
    html += "</style>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
    html += "<script>";
    html += "let temperatureData = []; let humidityData = []; let labels = [];";

    html += "function updateData() {";
    html += "  fetch('/data').then(response => response.json()).then(data => {";
    html += "    if (data.temperature !== null && data.humidity !== null) {";
    html += "      let now = new Date().toLocaleTimeString();";
    html += "      labels.push(now);";
    html += "      temperatureData.push(data.temperature);";
    html += "      humidityData.push(data.humidity);";
    html += "      if (labels.length > 20) { labels.shift(); temperatureData.shift(); humidityData.shift(); }";
    html += "      document.getElementById('tempValue').innerText = 'Temperature: ' + data.temperature + ' °C';";
    html += "      document.getElementById('humidValue').innerText = 'Humidity: ' + data.humidity + ' %';";
    html += "      myChart.update();";
    html += "    }";
    html += "  });";
    html += "}";
    html += "setInterval(updateData, 2000);"; // Update setiap 2 detik

    html += "document.addEventListener('DOMContentLoaded', function() {";
    html += "  const ctx = document.getElementById('myChart').getContext('2d');";
    html += "  window.myChart = new Chart(ctx, {";
    html += "    type: 'line',";
    html += "    data: {";
    html += "      labels: labels,";
    html += "      datasets: [";
    html += "        { label: 'Temperature (°C)', data: temperatureData, borderColor: 'rgba(255, 99, 132, 1)', backgroundColor: 'rgba(255, 99, 132, 0.2)', fill: true, tension: 0.4 },";
    html += "        { label: 'Humidity (%)', data: humidityData, borderColor: 'rgba(54, 162, 235, 1)', backgroundColor: 'rgba(54, 162, 235, 0.2)', fill: true, tension: 0.4 }";
    html += "      ]";
    html += "    },";
    html += "    options: {";
    html += "      responsive: true,";
    html += "      maintainAspectRatio: false,";
    html += "      scales: {";
    html += "        x: { title: { display: true, text: 'Time' } },";
    html += "        y: { title: { display: true, text: 'Value' }, beginAtZero: true }";
    html += "      }";
    html += "    }";
    html += "  });";
    html += "});";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Monitoring Temperatur dan Kelembapan</h1>";
    html += "<div id='chartContainer'><canvas id='myChart' width='400' height='200'></canvas></div>";
    html += "<div class='data-display'>";
    html += "<span id='tempValue'>Temperature: Loading...</span>";
    html += "<span id='humidValue'>Humidity: Loading...</span>";
    html += "</div>";
    html += "</div>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}


void setup() {
    Serial.begin(9600);
    dht.begin();

    // Aktifkan Access Point
    WiFi.softAP(soft_ap_name, ssid_password);
    Serial.println("Access Point started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Hubungkan ke Wi-Fi
    WiFi.begin(wifi_ssid, wifi_password);
    Serial.println("Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Konfigurasi server web
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();
}