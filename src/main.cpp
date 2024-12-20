#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define DHTPIN D6       // Pin data DHT
#define DHTTYPE DHT22   // Tipe sensor DHT

#define FIREBASE_HOST "https://project-iot-e5015-default-rtdb.asia-southeast1.firebasedatabase.app//" 
#define FIREBASE_AUTH "tiEyZ4q59u5QpfBVBgrEAZCv0JF5UF8WAgZRdfs1" 

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);

const char* soft_ap_name = "Kelembapan_Udara"; // Nama Wi-Fi Access Point
const char* ssid_password = "akulembab";      // Password Wi-Fi Access Point

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
    html += "p { font-size: 20px; color: #333; margin: 20px 0; }";
    html += "span { font-weight: bold; font-size: 24px; transition: all 0.3s ease-in-out; }";
    html += ".container { max-width: 600px; margin: 0 auto; padding: 20px; background-color: white; box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1); border-radius: 8px; }";
    html += "@media (max-width: 600px) { .container { width: 90%; } }";
    html += "</style>";
    html += "<script>";
    html += "function updateData() {";
    html += "  fetch('/data').then(response => response.json()).then(data => {";
    html += "    if (data.temperature !== null && data.humidity !== null) {";
    html += "      document.getElementById('temperature').innerText = data.temperature + ' °C';";
    html += "      document.getElementById('humidity').innerText = data.humidity + ' %';";
    html += "    } else {";
    html += "      document.getElementById('temperature').innerText = 'Error';";
    html += "      document.getElementById('humidity').innerText = 'Error';";
    html += "    }";
    html += "  }).catch(() => {";
    html += "    document.getElementById('temperature').innerText = 'Error';";
    html += "    document.getElementById('humidity').innerText = 'Error';";
    html += "  });";
    html += "}";
    html += "setInterval(updateData, 2000);"; // Update setiap 2 detik
    html += "</script>";
    html += "</head><body>";
    html += "<div class='container'>";
    html += "<h1>Monitoring Temperatur dan Kelembapan</h1>";
    html += "<p>Temperature: <span id='temperature'>Loading...</span></p>";
    html += "<p>Humidity: <span id='humidity'>Loading...</span></p>";
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

    // Konfigurasi server web
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();
}
