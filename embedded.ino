#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <DHT.h>


const char* ssid = "your_wifi_name";
const char* password = "your_wifi_password";

#define BOT_TOKEN "your_bot_token_api_from_botfather"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define DHT_TYPE DHT11
#define DHT_PIN 5
#define buzz 15
#define raindrop A0
#define echo 12
#define trig 14

DHT dht(DHT_PIN, DHT_TYPE);
  
unsigned long previousMillis = 0;
const long interval = 10000;

void setup() {
  Serial.begin(115200);
  pinMode(buzz, OUTPUT);
  pinMode(raindrop, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.println("Menghubungkan ke WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Membaca sensor hujan melalui DHT11
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    String rainStatus;

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Gagal membaca dari sensor DHT!");
    }

    if (temperature >= 20 && temperature <= 32 && humidity >= 70 && humidity <= 90) {
      rainStatus = "Kelembaban naik dan suhu turun. Potensi hujan.";
      tone(buzz, 300, 345);
      delay(700);
      tone(buzz, 300, 345);
      delay(700);
    } else {
      rainStatus = "Suhu dan kelembaban normal. Tidak berpotensi hujan.";
    }

    delay(2000);

    // Membaca curah hujan (raindrop)
    int curahHujan = map(analogRead(raindrop), 0, 1023, 0, 100);
    String rainIntensity;

  if (curahHujan > 80) {
    rainIntensity = "Tidak Hujan";
  } else if (curahHujan > 60 && curahHujan <= 80) {
    rainIntensity = "Hujan Gerimis";
  } else if (curahHujan <= 60) {
    rainIntensity = "Hujan Lebat";
    tone(buzz, 1000);
    delay(500);
    tone(buzz, 500);
    delay(500);
  }


    delay(2000);

    // Membaca ketinggian genangan air(ultrasonik)
    float ketinggianAir = cekGenangan();
    String floodStatus;

    if (ketinggianAir >= 45) {
      floodStatus = "Terjadi genangan tinggi. Cari tempat yang lebih tinggi sekarang juga!";
      tone(buzz, 1000);
      delay(500);
      tone(buzz, 500);
      delay(500);
      noTone(buzz);
      delay(500);
    } else if (ketinggianAir >= 35) {
      floodStatus = "Terjadi genangan cukup tinggi. Mulai mengungsi!";
      tone(buzz, 840);
      delay(500);
      tone(buzz, 420);
      delay(500);
      noTone(buzz);
      delay(500);
    } else if (ketinggianAir >= 25) {
      floodStatus = "Terjadi genangan sedang. Harap bersiap untuk mengungsi";
      tone(buzz, 720);
      delay(500);
      tone(buzz, 360);
      delay(500);
      noTone(buzz);
      delay(500);
    } else if (ketinggianAir >= 15) {
      floodStatus = "Terjadi genangan kecil. Harap waspada banjir";
      tone(buzz, 400);
      delay(500);
      tone(buzz, 200);
      delay(500);
      noTone(buzz);
      delay(500);
    } else {
      floodStatus = "Tidak ada genangan. Tidak perlu siaga banjir";
    }

    delay(2000);

    // Mengirim pesan status sensor ke Telegram
    String message = "Status Sensor:\n";
    message += "Temperatur: " + String(temperature) + "C\n";
    message += "Kelembaban: " + String(humidity) + "%\n";
    message += "Status Hujan: " + rainStatus + "\n";
    message += "Curah Hujan: " + rainIntensity + "\n";
    message += "Ketinggian Air: " + floodStatus;

    Serial.println("Mengirim pesan status sensor ke Telegram...");
    bool result = bot.sendMessage("1886480463", message);
    
    if(result) {
      Serial.println("Pesan status sensor terkirim ke Telegram");
    } else {
      Serial.println("Gagal mengirim pesan status sensor ke Telegram");
    }
  }
}

float cekGenangan() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long timer = pulseIn(echo, HIGH);
  float jarak = timer / 58.0;
  float ketinggianAir = 60 - jarak;
  return ketinggianAir;
}