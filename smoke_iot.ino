#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Informasi koneksi Wi-Fi
const char* ssid = "IoT"; // Ganti dengan nama Wi-Fi Anda
const char* password = "testingdoang"; // Ganti dengan password Wi-Fi Anda

// Informasi Telegram Bot
const char* botToken = "8198436698:AAEShtRDBH5ye639BgGM1HpN4XWP7cC7OuU"; // Ganti dengan Token Bot Anda
const char* chatID = "6273579245";     // Ganti dengan ID Chat Anda

WiFiClientSecure client; // Koneksi aman untuk HTTPS
UniversalTelegramBot bot(botToken, client);

// Pin untuk sensor, buzzer, dan LED
const int mq2Pin = 34;   // Pin analog untuk sensor MQ2
const int buzzerPin = 26; // Pin buzzer
const int ledPin = 25;    // Pin LED
const int threshold = 1000; // Ambang batas gas/asap

void sendTelegramAlert(int gasValue) {
  String message = "🚨 *ALERT*: Gas/Smoke Detected! 🚨\n";
  message += "Gas Value: " + String(gasValue) + "\n";
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
  pinMode(mq2Pin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // Mengizinkan koneksi HTTPS dengan sertifikat
  client.setInsecure(); // Hanya untuk pengujian, gunakan sertifikat yang valid untuk keamanan penuh

  Serial.println("MQ2 Gas/Smoke Detection with Telegram Notifications");
}

void loop() {
  int gasValue = analogRead(mq2Pin); // Membaca nilai analog dari sensor MQ2
  Serial.print("Gas/Smoke Value: ");
  Serial.println(gasValue);

  if (gasValue > threshold) {
    Serial.println("WARNING: Gas/Smoke Detected!");

    // Aktifkan buzzer dan LED
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, HIGH);

    // Kirim notifikasi Telegram
    sendTelegramAlert(gasValue);

    delay(10000); // Hindari spam notifikasi
  } else {
    // Matikan buzzer dan LED
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledPin, LOW);
  }

  delay(1000); // Tunggu 1 detik sebelum membaca lagi
}
