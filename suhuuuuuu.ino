#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>


// Deklarasi SSID dan Password WiFi
const char *ssid = "V1";
const char *password = "namakubento";


// Deklarasi MQTT Broker dan Topic
const char *mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char *mqtt_topic_temp = "/sensor/temp";
const char *mqtt_topic_hum = "/sensor/hum";


WiFiClient espClient;
PubSubClient client(espClient);


// Deklarasi Pin dan Tipe Sensor DHT
#define DHTPIN 4      // Sesuaikan pin yang terhubung dengan sensor DHT22 pada ESP32
#define DHTTYPE DHT11  // Tipe sensor DHT yang digunakan adalah DHT22
int LEDred = 13;

DHT dht(DHTPIN, DHTTYPE);


// Deklarasi Variabel untuk Pengiriman Data ke MQTT
unsigned long lastMsg = 0;
#define MSG_INTERVAL 2000  // Interval pengiriman data


// Method untuk Menghubungkan kembali ke MQTT jika Koneksi Terputus
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client1")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(1000);
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(LEDred, OUTPUT);


  // Koneksi ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }


  Serial.println("Connected to WiFi");


  // Set up MQTT client
  client.setServer(mqtt_server, mqtt_port);
  reconnect();


  // Inisialisasi DHT sensor
  dht.begin();
}


void loop() {
  // Memeriksa Koneksi MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Membaca Data dari Sensor DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  digitalWrite(LEDred, (temperature > 25) ? HIGH : LOW);

  // Memastikan pembacaan berhasil
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }


  // Mengirimkan Data Sensor ke MQTT
  unsigned long now = millis();
  if (now - lastMsg > MSG_INTERVAL) {
    lastMsg = now;


    // Mengirim data suhu
    char tempStr[8];
    dtostrf(temperature, 1, 2, tempStr);
    client.publish(mqtt_topic_temp, tempStr);
    Serial.print("Temperature sent to MQTT: ");
    Serial.println(tempStr);


    // Mengirim data kelembapan
    char humStr[8];
    dtostrf(humidity, 1, 2, humStr);
    client.publish(mqtt_topic_hum, humStr);
    Serial.print("Humidity sent to MQTT: ");
    Serial.println(humStr);
  }

  delay(2000);
}