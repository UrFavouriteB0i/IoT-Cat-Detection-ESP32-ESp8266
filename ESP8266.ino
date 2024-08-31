// WiFi credentials
const char* WIFI_SSID = "CHANGE YOUR WIFI SSID";
const char* WIFI_PASS = "CHANGE YOUR WIFI PASS";

// replace with your bot token and chat id
#define TELEGRAM_TOKEN "CHANGE YOUR TELEGRAM TOKEN"
#define TELEGRAM_CHAT "CHANGE YOUR TELEGRAM CHAT ID"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyStepper_28BYJ_48.h>

WiFiClientSecure client;
UniversalTelegramBot bot(TELEGRAM_TOKEN, client);

//led and button definition
const int led1 = 10;
const int led2 = 9;
const int bttn = 8;

bool LED_PIN_STATE = false;
bool BUTTON_STATE = false;
bool STEPPER_STATE = false;
unsigned long stepperWaitTime = 5000;

//ultrasonic definition
const int trigpin = 14;
const int echopin = 12;
long duration, cm;

//Stepper definition
const int MOTOR_IN1_PIN = 5;
const int MOTOR_IN2_PIN = 4;
const int MOTOR_IN3_PIN = 0;
const int MOTOR_IN4_PIN = 2;

TinyStepper_28BYJ_48 stepper;

//Feeder Timer Interval
unsigned long lastFeedTime = 0;
unsigned long feedInterval = 0; // Time in milliseconds

//Status Timer Interval
unsigned long lastUpdateTime = 0;
unsigned long statusInterval = 0; //Time in milliseconds

String lastRotationTime = "";

//previous (-5) command variable
String history = "";

String led = "";
String button = "";
String US = "";
String Step = "";

void ledUpdate(){
  digitalWrite(led1, HIGH);
  delay (50);
  digitalWrite(led1,LOW);
}

void handleNewMessages(int numNewMessages) {
  ledUpdate();
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
    if (text == "/start"){
      String welcome = "Selamat datang, " + from_name + "!\n";
      welcome += "Gunakan perintah yang ada untuk berinteraksi dengan Automatic Cat Feeder 🐱\n";
      bot.sendMessage(TELEGRAM_CHAT, welcome, "");
    }
    if (text == "/help") {
      String Help = "Halo, " + from_name + "\n";
      Help += "List command yang bisa digunakan pada Automatic Cat Feeder\n";
      Help += "/Photo : Ambil foto dan status deteksi kucing didepan alat\n";
      Help += "/Feed  : Memberikan makan secara manual\n";
      Help += "/FoodCheck : Memeriksa jumlah makanan yang tersisa di feeder\n";
      Help += "/Status  : Menampilkan status dari semua modul dan fungsi\n";
      Help += "/SetTimerFeed {angka+s/m/h} : Mengatur timer untuk mengeluarkan makanan pada interval yang ditentukan\n";
      Help += "/CheckConnection : Memeriksa status koneksi Wi-Fi\n";
      Help += "/CheckCommand  : Menampilkan dafter perintah terbaru yang diterima\n";
      Help += "/CheckModule : Memeriksa status dari Modul ESP32-Cam dan ESP8266\n";
      Help += "SetTimerStatus {angka+s/m/h}: Mengatur timer untuk pembaruan status";
      bot.sendMessage(TELEGRAM_CHAT, Help, "");
    }
    if (text == "/photo"){
      continue;
    }
    if (text == "/feed"){
      bot.sendMessage(TELEGRAM_CHAT, "🍽 Pakan telah diberikan", "");
      RotateStepper();
      history = text;
    }
    if (text == "/foodcheck"){
      String container = checkUS();
      bot.sendMessage(TELEGRAM_CHAT, container, "");
      history = text;
    }
    if(text == "/status"){
      String Stats = statsChecker();
      bot.sendMessage(TELEGRAM_CHAT, Stats, "");
      history = text;
    }
    if (text == "/settimerfeed"){
      String FeederTimer = bot.messages[i].text.substring(14);
      setFeedTimer(FeederTimer);
      history = text;
    }
    if (text == "/checkconnection"){
      if(WiFi.status() == WL_CONNECTED){
        bot.sendMessage(TELEGRAM_CHAT, "✅ Wi-Fi Status: OK", "");
      }
      history = text;
    }
    if (text == "/checkcommand"){
      if (history == ""){
        continue;
      }else{
      String prev = "Command terakhir adalah: " + history;
      bot.sendMessage(TELEGRAM_CHAT, prev, "");}
    }
    if (text == "/checkmodule"){
      moduleChecker();
      String mod = "🔌 LED status: " + led + "\n";
      mod += "🔘 Button status: " + button + "\n";
      mod += "📏 Ultrasonic status: " + US + "\n";
      mod += "⚙ Stepper status: " + Step;
      bot.sendMessage(TELEGRAM_CHAT, mod, "");
      history = text;
    }
    
    if (text == "/settimerstatus"){
      String statusTimer = bot.messages[i].text.substring(14);
      setStatsTimer(statusTimer);
      history = text;
    }
  }
}


void moduleChecker(){ 
  // Assume LED_PIN_STATE and BUTTON_STATE are global variables
  LED_PIN_STATE = !LED_PIN_STATE;
  digitalWrite(led1, LED_PIN_STATE);
  delay(100); // Added delay for stability
  LED_PIN_STATE = !LED_PIN_STATE;
  digitalWrite(led1, LED_PIN_STATE);

  // Check LED status
  if (LED_PIN_STATE != LED_PIN_STATE) {
    led = "OK";
  } else {
    led = "NOT OK";
  }

  // Check Button status
  BUTTON_STATE = !BUTTON_STATE;
  if (BUTTON_STATE != BUTTON_STATE) {
    button = "OK";
  } else {
    button = "NOT OK";
  }

  // Ultrasonic sensor check
  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(5);
  duration = pulseIn(echopin, HIGH);
  cm = duration / 58.2;

  if (cm == 0) {
    US = "NOT OK";
  } else if (cm > 0) {
    US = "OK";
  }

  // Check Stepper status
  if (!checkStepperConnected) {
    Step = "NOT OK";
  } else {
    Step = "OK";
  }
}

bool checkStepperConnected() {
  unsigned long startTime = millis();

  // Start the motor
  stepper.setSpeedInStepsPerSecond(256); 
  stepper.moveRelativeInSteps(-100);

  // Check if the motor movement took too long
  if (millis() - startTime > stepperWaitTime) {
    return false; // The motor took too long to move
  }

  return true; // The motor moved within the expected time
}

void setFeedTimer(String FeederTimer){
  String satuan;
  feedInterval = FeederTimer.toInt();
  char unit = FeederTimer.charAt(FeederTimer.length()-1);
  if (unit == 's') {
    satuan = " detik";
    feedInterval = feedInterval * 1000;  // Convert seconds to milliseconds
  } 
  else if (unit == 'm') {
    satuan = " menit";
    feedInterval = feedInterval * 60000; // Convert minutes to milliseconds
  } 
  else if (unit == 'h') {
    satuan = " jam";
    feedInterval = feedInterval * 3600000; // Convert hours to milliseconds
  }
  else {
    bot.sendMessage(TELEGRAM_CHAT, "Invalid unit. Use 's' untuk detik, 'm' untuk menit, or 'h' for jam.", "");
  }
  String output = "Pemberian makan diatur menjadi " + FeederTimer + satuan + " sekali";
  bot.sendMessage(TELEGRAM_CHAT, output, "");
}

void setStatsTimer(String statsTimer){
  String satuan;
  statusInterval = statsTimer.toInt();
  char unit = statsTimer.charAt(statsTimer.length()-1);
  if (unit == 's') {
    satuan = " detik";
    statusInterval = statusInterval * 1000;  // Convert seconds to milliseconds
  } 
  else if (unit == 'm') {
    satuan = " menit";
    statusInterval = statusInterval * 60000; // Convert minutes to milliseconds
  } 
  else if (unit == 'h') {
    satuan = " jam";
    statusInterval = statusInterval * 3600000; // Convert hours to milliseconds
  }
  else {
    bot.sendMessage(TELEGRAM_CHAT, "Invalid unit. Use 's' untuk detik, 'm' untuk menit, or 'h' for jam.", "");
  }
  String output = "Update status diatur menjadi " + statsTimer + satuan + " sekali";
  bot.sendMessage(TELEGRAM_CHAT, output, ""); 
}

String statsChecker(){
  String stats = "Automatic Cat Feeder Status:\n";
  stats += "🔘 Button OK\n";
  stats += "🔌 LED OK\n";
  stats += checkUS()+"\n";
  stats += "🕒 Feeder terakhir berjalan pada: " + lastRotationTime;
  
  return stats;
}

void RotateStepper(){
  Serial.println("moving stepper");
  stepper.setSpeedInStepsPerSecond(512);
  stepper.setAccelerationInStepsPerSecondPerSecond(512);
  stepper.moveRelativeInSteps(-314);
  Serial.println("reversing");
  delay(500);
  stepper.moveRelativeInSteps(2048 * 10);
//  DateTime now = rtc.now();
//  lastRotationTime = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
}

String checkUS(){
  String kondisi;
  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(5);
  duration = pulseIn(echopin, HIGH);
  cm = duration /58.2;
  if(cm < 5){
    kondisi = "Stok makanan penuh";
  }
  else if (5 < cm <10){
    kondisi = "Makanan masih cukup";
  }
  else if (cm > 10){
    kondisi = "Stok makanan hampir habis";
  }
  return kondisi;
}

void setup(){
  Serial.begin(115200);
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(bttn, INPUT);
  stepper.connectToPins(MOTOR_IN1_PIN, MOTOR_IN2_PIN, MOTOR_IN3_PIN, MOTOR_IN4_PIN);
  
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  bot.sendMessage(TELEGRAM_CHAT, "☀ ESP8266 telah aktif dan siap digunakan!", "");
}

void loop(){
  if (WiFi.status() == WL_CONNECTED)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      yield();
    }
  }
  // Check if it's time to run the feeding mechanism
  if (feedInterval > 0 && millis() - lastFeedTime >= feedInterval) {
    RotateStepper();
    lastFeedTime = millis();
    yield();
  }
  // Check if it's time to run the status update
  if (feedInterval > 0 && millis() - lastFeedTime >= feedInterval) {
    statsChecker();
    lastUpdateTime = millis();
    yield();
  }
}
