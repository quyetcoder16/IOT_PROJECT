#define BLYNK_TEMPLATE_ID "TMPL6J1VBPX2h"
#define BLYNK_TEMPLATE_NAME "IOT102"
#define BLYNK_AUTH_TOKEN "K_XzincbeArURJ5JMohS3yOw_4zM-2cr"

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Thay bằng token từ Blynk
char ssid[] = "Q2";         // Thay bằng SSID WiFi
char pass[] = "2345678901"; // Thay bằng mật khẩu WiFi

// Định nghĩa Virtual Pins
#define VPIN_MODE V0         // Chế độ tự động/tay
#define VPIN_LAMP_OUTDOOR V1 // Đèn ngoài sân
#define VPIN_LAMP_LIVING V2  // Đèn phòng khách
#define VPIN_LAMP3 V3        // Đèn 3
#define VPIN_FAN V4          // Quạt
#define VPIN_SERVO_OPEN V5   // Mở khóa qua app
#define VPIN_TEMP V6         // Nhiệt độ
#define VPIN_HUMI V7         // Độ ẩm
#define VPIN_SERVO_STATUS V8 // Trạng thái servo

// Định nghĩa chân
#define SS_PIN 5
#define RST_PIN 0
#define RELAY_LAMP_OUTDOOR 16
#define RELAY_LAMP_LIVING 17
#define RELAY_LAMP3 21
#define RELAY_FAN 22
#define BUTTON_MODE 32
#define BUTTON_LAMP_OUTDOOR 33
#define BUTTON_LAMP_LIVING 15
#define BUTTON_LAMP3 2
#define BUTTON_FAN 13
#define DHTPIN 4
#define DHTTYPE DHT22
#define LDR_PIN 34
#define PIR_OUTDOOR 25
#define PIR_LIVING 26
#define BUZZER 14
#define SERVO_PIN 12

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[4];
DHT dht(DHTPIN, DHTTYPE);
Servo servo;

bool autoMode = false; // Chế độ tự động (false = tay/app)
unsigned long lastDebounce[5] = {0, 0, 0, 0, 0};
const int debounceDelay = 200;
unsigned long lastDHTUpdate = 0;
const int dhtUpdateInterval = 2000;
unsigned long lastServoMove = 0;
unsigned long lastOutdoor = 0;
unsigned long lastLiving = 0;
unsigned long lastBuzzer = 0;
int wrongCardCount = 0;
const byte validCard[4] = {0x97, 0xBE, 0x70, 0x62}; // UID thẻ mở cửa: 97 BE 70 62

void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

BLYNK_WRITE(VPIN_MODE)
{ // Chế độ tự động/tay qua app
  autoMode = param.asInt();
  Serial.print("Che do qua app: ");
  Serial.println(autoMode ? "AUTO" : "MANUAL");
}

BLYNK_WRITE(VPIN_LAMP_OUTDOOR)
{ // Đèn ngoài sân qua app
  if (!autoMode)
  {
    digitalWrite(RELAY_LAMP_OUTDOOR, param.asInt() ? LOW : HIGH);
    Serial.print("Den ngoai san qua app: ");
    Serial.println(param.asInt() ? "ON" : "OFF");
  }
}

BLYNK_WRITE(VPIN_LAMP_LIVING)
{ // Đèn phòng khách qua app
  if (!autoMode)
  {
    digitalWrite(RELAY_LAMP_LIVING, param.asInt() ? LOW : HIGH);
    Serial.print("Den phong khach qua app: ");
    Serial.println(param.asInt() ? "ON" : "OFF");
  }
}

BLYNK_WRITE(VPIN_LAMP3)
{ // Đèn 3 qua app
  if (!autoMode)
  {
    digitalWrite(RELAY_LAMP3, param.asInt() ? LOW : HIGH);
    Serial.print("Den 3 qua app: ");
    Serial.println(param.asInt() ? "ON" : "OFF");
  }
}

BLYNK_WRITE(VPIN_FAN)
{ // Quạt qua app
  if (!autoMode)
  {
    digitalWrite(RELAY_FAN, param.asInt() ? LOW : HIGH);
    Serial.print("Quat qua app: ");
    Serial.println(param.asInt() ? "ON" : "OFF");
  }
}

BLYNK_WRITE(VPIN_SERVO_OPEN)
{ // Mở khóa qua app
  servo.write(90);
  Serial.println("Servo qua app: OPEN");
  Blynk.virtualWrite(VPIN_SERVO_STATUS, 1); // Cập nhật trạng thái mở
  lastServoMove = millis();
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();
  for (byte i = 0; i < 6; i++)
  {
    key.keyByte[i] = 0xFF;
  }

  dht.begin();
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(0);
  Blynk.virtualWrite(VPIN_SERVO_STATUS, 0); // Trạng thái ban đầu: đóng

  pinMode(RELAY_LAMP_OUTDOOR, OUTPUT);
  pinMode(RELAY_LAMP_LIVING, OUTPUT);
  pinMode(RELAY_LAMP3, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_LAMP_OUTDOOR, HIGH);
  digitalWrite(RELAY_LAMP_LIVING, HIGH);
  digitalWrite(RELAY_LAMP3, HIGH);
  digitalWrite(RELAY_FAN, HIGH);

  pinMode(BUTTON_MODE, INPUT_PULLUP);
  pinMode(BUTTON_LAMP_OUTDOOR, INPUT_PULLUP);
  pinMode(BUTTON_LAMP_LIVING, INPUT_PULLUP);
  pinMode(BUTTON_LAMP3, INPUT_PULLUP);
  pinMode(BUTTON_FAN, INPUT_PULLUP);

  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_OUTDOOR, INPUT);
  pinMode(PIR_LIVING, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Serial.println(F("San sang!"));
}

void loop()
{
  Blynk.run();

  // Kiểm tra RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    bool cardValid = true;
    for (byte i = 0; i < 4; i++)
    {
      if (rfid.uid.uidByte[i] != validCard[i])
      {
        cardValid = false;
        break;
      }
    }
    if (cardValid)
    {
      Serial.println("The RFID: DUNG - Mo khoa");
      servo.write(90);
      Blynk.virtualWrite(VPIN_SERVO_STATUS, 1); // Cập nhật trạng thái mở
      lastServoMove = millis();
      wrongCardCount = 0;
    }
    else
    {
      Serial.println("The RFID: SAI");
      wrongCardCount++;
      if (wrongCardCount >= 3)
      {

        digitalWrite(BUZZER, HIGH);
        lastBuzzer = millis();
        Serial.println("Sai 3 lan - Bao dong!");
      }
    }
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Đóng servo sau 2 giây và cập nhật trạng thái
  if (lastServoMove > 0 && millis() - lastServoMove > 5000)
  {
    servo.write(0);
    Serial.println("Servo: CLOSED");
    Blynk.virtualWrite(VPIN_SERVO_STATUS, 0); // Cập nhật trạng thái đóng
    lastServoMove = 0;
  }

  // Tắt còi sau 1 giây
  if (lastBuzzer > 0 && millis() - lastBuzzer > 1000)
  {
    digitalWrite(BUZZER, LOW);
    lastBuzzer = 0;
  }

  // Công tắc chế độ (vật lý)
  if (digitalRead(BUTTON_MODE) == LOW && (millis() - lastDebounce[0] > debounceDelay))
  {
    autoMode = !autoMode;
    Serial.print("Che do qua tay: ");
    Serial.println(autoMode ? "AUTO" : "MANUAL");
    Blynk.virtualWrite(VPIN_MODE, autoMode); // Đồng bộ với Blynk
    lastDebounce[0] = millis();
  }

  // Chế độ tự động
  if (autoMode)
  {
    float temp = dht.readTemperature();
    int ldrValue = analogRead(LDR_PIN);
    bool pirOutdoor = digitalRead(PIR_OUTDOOR);
    bool pirLiving = digitalRead(PIR_LIVING);

    if (!isnan(temp) && temp > 32)
    {
      digitalWrite(RELAY_FAN, LOW);
      Serial.println("Nhiet do > 32 - Quat: ON");
    }
    else
    {
      digitalWrite(RELAY_FAN, HIGH);
    }

    if (ldrValue < 200 && pirOutdoor)
    { // Trời tối (giá trị LDR cần thử nghiệm)
      digitalWrite(RELAY_LAMP_OUTDOOR, LOW);
      lastOutdoor = millis();
      Serial.println("Troi toi + Chuyen dong ngoai san - Den ngoai san: ON");
    }
    else
    {
      if (lastOutdoor > 0 && millis() - lastOutdoor > 5000)
      {
        digitalWrite(RELAY_LAMP_OUTDOOR, HIGH);
      }
    }

    if (ldrValue < 200 && pirLiving)
    {
      digitalWrite(RELAY_LAMP_LIVING, LOW);
      Serial.println("Troi toi + Chuyen dong phong khach - Den phong khach: ON");
      lastLiving = millis();
    }
    else
    {
      if (lastLiving > 0 && millis() - lastLiving > 5000)
      {
        digitalWrite(RELAY_LAMP_LIVING, HIGH);
      }
    }
  }
  else
  { // Chế độ tay
    if (digitalRead(BUTTON_LAMP_OUTDOOR) == LOW && (millis() - lastDebounce[1] > debounceDelay))
    {
      digitalWrite(RELAY_LAMP_OUTDOOR, !digitalRead(RELAY_LAMP_OUTDOOR));
      Blynk.virtualWrite(VPIN_LAMP_OUTDOOR, digitalRead(RELAY_LAMP_OUTDOOR) ? 0 : 1);
      Serial.print("Den ngoai san qua tay: ");
      Serial.println(digitalRead(RELAY_LAMP_OUTDOOR) ? "OFF" : "ON");
      lastDebounce[1] = millis();
    }
    if (digitalRead(BUTTON_LAMP_LIVING) == LOW && (millis() - lastDebounce[2] > debounceDelay))
    {
      digitalWrite(RELAY_LAMP_LIVING, !digitalRead(RELAY_LAMP_LIVING));
      Blynk.virtualWrite(VPIN_LAMP_LIVING, digitalRead(RELAY_LAMP_LIVING) ? 0 : 1);
      Serial.print("Den phong khach qua tay: ");
      Serial.println(digitalRead(RELAY_LAMP_LIVING) ? "OFF" : "ON");
      lastDebounce[2] = millis();
    }
    if (digitalRead(BUTTON_LAMP3) == LOW && (millis() - lastDebounce[3] > debounceDelay))
    {
      digitalWrite(RELAY_LAMP3, !digitalRead(RELAY_LAMP3));
      Blynk.virtualWrite(VPIN_LAMP3, digitalRead(RELAY_LAMP3) ? 0 : 1);
      Serial.print("Den 3 qua tay: ");
      Serial.println(digitalRead(RELAY_LAMP3) ? "OFF" : "ON");
      lastDebounce[3] = millis();
    }
    if (digitalRead(BUTTON_FAN) == LOW && (millis() - lastDebounce[4] > debounceDelay))
    {
      digitalWrite(RELAY_FAN, !digitalRead(RELAY_FAN));
      Blynk.virtualWrite(VPIN_FAN, digitalRead(RELAY_FAN) ? 0 : 1);
      Serial.print("Quat qua tay: ");
      Serial.println(digitalRead(RELAY_FAN) ? "OFF" : "ON");
      lastDebounce[4] = millis();
    }
  }

  // Cập nhật nhiệt độ, độ ẩm lên Blynk mỗi 2 giây
  if (millis() - lastDHTUpdate > dhtUpdateInterval)
  {
    float temp = dht.readTemperature();
    float humi = dht.readHumidity();
    if (!isnan(temp) && !isnan(humi))
    {
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.print(" °C    Humidity: ");
      Serial.print(humi);
      Serial.println(" %");
      Blynk.virtualWrite(VPIN_TEMP, temp); // Gửi nhiệt độ
      Blynk.virtualWrite(VPIN_HUMI, humi); // Gửi độ ẩm
    }
    else
    {
      Serial.println("Failed to read from DHT22 sensor!");
    }
    lastDHTUpdate = millis();
  }
}

void printHex(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}