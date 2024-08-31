// WiFi credentials
#define WIFI_SSID "CHANGE YOUR WIFI SSID"
#define WIFI_PASS "CHANGE YOUR WIFI PASSWORD"
// replace with your bot token and chat id
#define TELEGRAM_TOKEN "CHANGE YOUR TELEGRAM TOKEN"
#define TELEGRAM_CHAT "CHANGE YOUR TELEGRAM CHAT ID"

#include <kucing-clas_inferencing.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/edgeimpulse/fomo.h>
#include <eloquent_esp32cam/extra/esp32/telegram.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//Telegram by Eloquent
using eloq::camera;
using eloq::ei::fomo;
using eloq::wifi;
using eloq::telegram;
//FOMO Object Detection by Eloquent


WiFiClientSecure clientTCP;
UniversalTelegramBot bot(TELEGRAM_TOKEN, clientTCP);
bool sendPhoto = false;
bool AIflow = false;
bool flashState = false;

#define FLASH_LED_PIN 4

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages){
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != TELEGRAM_CHAT){
      bot.sendMessage(TELEGRAM_CHAT, "Unauthorized user", "");
      continue;
    }
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/Start"){
      String welcome = "Selamat datang, " + from_name + "!\n";
      welcome += "Gunakan perintah untuk berinteraksi dengan ESP32-CAM 📷\n";
      if (telegram.to(TELEGRAM_CHAT).send(welcome).isOk()){
      Serial.println("Welcome message terkirim");
      }
      delay(1000);
    }
    if (text == "/Photo") {
      sendPhoto = true;
      Serial.println("Permintaan foto baru");
    }
    if (text == "/AI"){
      checkAI();
    }
    if (text == "/Status"){
      sendPhoto = true;
      Serial.println("Status terbaru terkirim");
      delay(500);
    }
    if (text == "/CheckConnection") {
      if (wifi.connect().isOk()) {
        String conn = "✅ Koneksi Wi-Fi: OK";
        if (telegram.to(TELEGRAM_CHAT).send(conn).isOk()){
        Serial.println("Status koneksi terkirim");
        }
      delay(500);
      }
    }
    if (text == "/CheckModule") {
      String mod = "📷 Modul ESP32: OK";
      if (telegram.to(TELEGRAM_CHAT).send(mod).isOk()){
      Serial.println("Status module terkirim");
      }
      delay(500);  // Menambahkan delay agar tidak tabrakan dengan ESP8266
    }
    if (text.equals("/Feed") || text.equals("/SetTimerFeed") || text.equals("/FoodCheck") || text.equals("/CheckCommand") || text.equals("/SetTimerStatus") || text.equals("/Help")){
      //nothing
    }
  }
}

//SETUP INITIALIZATION
void setup() {
    delay(3000);
    Serial.begin(115200);
    Serial.println("GAMBAR TELEGRAM");

    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, flashState);
    // camera settings
    // replace with your own model!
    camera.pinout.aithinker();
    camera.brownout.disable();
//    camera.resolution.vga();
//    camera.quality.high();
    
    // init camera
    while (!camera.begin().isOk())
        Serial.println(camera.exception.toString());

    // connect to WiFi
    while (!wifi.connect().isOk())
      Serial.println("Connecting to wifi...");
      
    clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    
    // connect to Telegram API
    while (!telegram.begin().isOk())
      Serial.println("Connecting to Telegram...");

    String text = "📷 Kamera ESP32 Cam telah aktif dan siap digunakan!";
    if (telegram.to(TELEGRAM_CHAT).send(text).isOk()) {
      Serial.println("Camera OK");
      Serial.println("Telegram OK");
      Serial.println("FOMO Model OK");
    }
}

void loop() {
  if(sendPhoto){
    sendPhotoTelegram();
    checkAI();
  }
  //HANDLING NEW MESSAGES
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

//SENDING PHOTO USING ELOQUENT LIBRARY
void sendPhotoTelegram(){
  camera.resolution.vga();
  camera.quality.high();
//  digitalWrite(FLASH_LED_PIN, HIGH);
  if (!camera.capture().isOk()) {
        Serial.println(camera.exception.toString());
        return;
  }
//  digitalWrite(FLASH_LED_PIN, LOW);
  // send
  if (telegram.to(TELEGRAM_CHAT).send(camera.frame).isOk()){
      Serial.println("Foto berhasil dikirim ke Telegram");
      sendPhoto = false;
      }
  else
      Serial.println(telegram.exception.toString());
}

void checkAI(){
  // capture picture
  camera.resolution.yolo();
  camera.pixformat.rgb565();
  if (!camera.capture().isOk()) {
      Serial.println(camera.exception.toString());
      return;
  }
  // run FOMO
  if (!fomo.run().isOk()) {
    Serial.println(fomo.exception.toString());
    return;
  }
  
   if (!fomo.foundAnyObject()){
     String text = "🐱X Tidak terdeteksi Objek: Kucing";
      if (telegram.to(TELEGRAM_CHAT).send(text).isOk()){
         Serial.println("Hasil klasifikasi AI berhasil dikirim ke Telegram");
         return;
        }
     }
   if (fomo.first.proba <0.65){
    String text = "🐱X Tidak terdeteksi Objek: Kucing";
      if (telegram.to(TELEGRAM_CHAT).send(text).isOk()){
         Serial.println("Hasil klasifikasi AI berhasil dikirim ke Telegram");
         return;
        }
   }

  String text = "🐱! Berhasil Terdeteksi Objek: " + fomo.first.label + ", dengan kemungkinan: " + String(fomo.first.proba * 100, 2) + "%";
  if (telegram.to(TELEGRAM_CHAT).send(text).isOk()){
      Serial.println("Hasil klasifikasi AI berhasil dikirim ke Telegram");
      return;
  }
  else
      Serial.println(telegram.exception.toString());
}
