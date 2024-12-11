#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <MQUnifiedsensor.h>

// Informasi koneksi Wi-Fi
const char* ssid = "IoT"; // Ganti dengan nama Wi-Fi Anda
const char* password = "testingdoang"; // Ganti dengan password Wi-Fi Anda

// Informasi Telegram Bot
const char* botToken = "8198436698:AAEShtRDBH5ye639BgGM1HpN4XWP7cC7OuU"; // Ganti dengan Token Bot Anda
const char* chatID = "6273579245"; // Ganti dengan ID Chat Anda

WiFiClientSecure client; // Koneksi aman untuk HTTPS
UniversalTelegramBot bot(botToken, client);

// Pin untuk sensor, buzzer, dan LED
const int mq2Pin = 33;   // Pin analog untuk sensor MQ2
const int buzzerPin = 26; // Pin buzzer
const int ledPin = 25;    // Pin LED
const float threshold = 10.0; // Ambang batas gas/asap

// Parameter MQ2
#define Board "ESP32"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12
#define RatioMQ2CleanAir 9.83 // Rasio standar MQ2 dalam udara bersih
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, mq2Pin, "MQ-2");
float prevPpm = 0; // Variabel untuk nilai PPM sebelumnya
float fluctuation = 0; // Variabel untuk menghitung fluktuasi


void sendTelegramAlert(const String& type, float ppm) {
  String message = "🚨*ALERT*: " + type + " Detected! 🚨\n";
  message += "PPM Value: " + String(ppm, 2) + "\n";
  message += "Please take necessary precautions!";

  if (bot.sendMessage(chatID, message, "Markdown")) {
    Serial.println("Telegram alert sent!");
  } else {
    Serial.println("Failed to send Telegram alert.");
  }
}

void setup() {
  Serial.begin(115200);

  // Koneksi ke Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");

  // Inisialisasi pin
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(ledPin, LOW);

/*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  // Konfigurasi sensor MQ2
  MQ2.setRegressionMethod(1); // Gunakan regresi linear
  MQ2.setA(574.25);           // Nilai konstanta A (bisa berbeda untuk setiap sensor)
  MQ2.setB(-2.222);           // Nilai konstanta B
  MQ2.init();

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0/10);
  Serial.println(calcR0);

  // Mengizinkan koneksi HTTPS dengan sertifikat
  client.setInsecure(); // Hanya untuk pengujian, gunakan sertifikat yang valid untuk keamanan penuh

  Serial.println("MQ2 Gas/Smoke Detection with Telegram Notifications");
}

void loop() {
  MQ2.update(); // Update nilai sensor
  float ppm = MQ2.readSensor(); // Membaca nilai gas dalam satuan PPM

  fluctuation = abs(ppm - prevPpm); // Menghitung fluktuasi
  prevPpm = ppm; // Menyimpan nilai PPM sebelumnya

  Serial.print("Gas/Smoke Value (PPM): ");
  Serial.println(ppm);

  if (ppm > threshold) {
    if (fluctuation > 3) { // Asap cenderung fluktuasi besar
      Serial.println("WARNING: Smoke Detected!");
      sendTelegramAlert("Smoke", ppm);
    } else {
      Serial.println("WARNING: Gas Detected!");
      sendTelegramAlert("Gas", ppm);
    }

    // Aktifkan buzzer dan LED
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, HIGH);

    delay(5000); // Hindari spam notifikasi
  } else {
    // Matikan buzzer dan LED
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, LOW);
  }

  delay(1000); // Tunggu 1 detik sebelum membaca lagi
}
