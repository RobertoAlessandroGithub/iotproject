#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseESP8266.h>
#include <Adafruit_Sensor.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DHTPIN D6       // Pin data DHT
#define DHTTYPE DHT22   // Tipe sensor DHT

// Firebase Config
#define FIREBASE_HOST "https://project-iot-e5015-default-rtdb.asia-southeast1.firebasedatabase.app" // URL Firebase Anda
#define FIREBASE_AUTH "AIzaSyCjlxTScbXabPFJlSYJVXt5tLHyJPxKYjg"          // Secret Firebase Anda

#define WIFI_SSID "Febrian" //WiFi SSID to which you want NodeMCU to connect
#define WIFI_PASSWORD "nevermore" //Password of your wifi network

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);    

// Inisialisasi Firebase
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void handleData() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Jika pembacaan gagal, kirimkan nilai error
    if (isnan(temperature) || isnan(humidity)) {
        server.send(500, "application/json", "{\"temperature\":null,\"humidity\":null}");
        return;
    }

    // Kirim data JSON ke browser
    String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    server.send(200, "application/json", json);

    // Membuat timestamp sebagai ID unik untuk setiap data
    String timestamp = String(millis());

    // Kirim data ke Firebase dengan path dinamis berdasarkan timestamp
    String tempPath = "/sensor/temperature/" + timestamp;
    if (Firebase.setFloat(firebaseData, tempPath, temperature)) {
        Serial.println("Data temperatur terkirim ke Firebase!");
    } else {
        Serial.print("Gagal mengirim data temperatur: ");
        Serial.println(firebaseData.errorReason());
    }

    String humidPath = "/sensor/humidity/" + timestamp;
    if (Firebase.setFloat(firebaseData, humidPath, humidity)) {
        Serial.println("Data kelembapan terkirim ke Firebase!!");
    } else {
        Serial.print("Gagal mengirim data kelembapan: ");
        Serial.println(firebaseData.errorReason());
    }
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
    html += "      document.getElementById('tempValue').innerText = 'Temperature: ' + data.temperature + ' \u00B0C';";
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
    html += "        { label: 'Temperature (\u00B0C)', data: temperatureData, borderColor: 'rgba(255, 99, 132, 1)', backgroundColor: 'rgba(255, 99, 132, 0.2)', fill: true, tension: 0.4 },";
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
    Serial.begin(9600); // Select the same baud rate if you want to see the datas on Serial Monitor
    Serial.println("Serial communication started\n\n");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // try to connect with wifi 
    Serial.print("Connecting to ");
    Serial.print(WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println();
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP Address is: ");
    Serial.println(WiFi.localIP()); // print local IP address

    /* Assign the database URL and database secret(required) */
    config.database_url = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
    delay(1000);

    // Konfigurasi server web
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();
}
