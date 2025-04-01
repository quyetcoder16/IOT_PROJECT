// #define BLYNK_TEMPLATE_ID "TMPL6J1VBPX2h"
// #define BLYNK_TEMPLATE_NAME "IOT102"
// #define BLYNK_AUTH_TOKEN "K_XzincbeArURJ5JMohS3yOw_4zM-2cr"

// #include <WiFi.h>
// #include <BlynkSimpleEsp32.h>
// #include <DHT.h>
// #include <SPI.h>
// #include <MFRC522.h>
// #include <ESP32Servo.h>

// // Thông tin WiFi
// char ssid[] = "Q2";
// char pass[] = "2345678901";

// // Cấu hình chân
// #define DHTPIN 4
// #define DHTTYPE DHT11
// #define RELAY_LAMP_OUTDOOR 16 // Đèn ngoài sân
// #define RELAY_LAMP_LIVING 17  // Đèn phòng khách
// #define RELAY_LAMP3 18        // Đèn 3
// #define RELAY_FAN 19          // Quạt
// #define LDR_PIN 34
// #define PIR_OUTDOOR 26
// #define PIR_LIVING 25
// #define RFID_SDA 5
// #define RFID_RST 27
// #define SERVO_PIN 13
// #define BUZZER 22
// #define BUTTON_MODE 32
// #define BUTTON_LAMP_OUTDOOR 33
// #define BUTTON_LAMP_LIVING 15
// #define BUTTON_LAMP3 2
// #define BUTTON_FAN 0

// // Virtual pins cho Blynk
// #define VPIN_LAMP_OUTDOOR V1
// #define VPIN_LAMP_LIVING V2
// #define VPIN_LAMP3 V3
// #define VPIN_FAN V4
// #define VPIN_DOOR V5
// #define VPIN_TEMP V6
// #define VPIN_HUMI V7
// #define VPIN_MODE V8

// DHT dht(DHTPIN, DHTTYPE);
// MFRC522 rfid(RFID_SDA, RFID_RST);
// Servo doorServo;

// int wrongCardCount = 0;
// String validCard = "A1B2C3D4"; // Thay bằng UID thẻ RFID của bạn
// unsigned long lastUpdate = 0;
// unsigned long lastDebounce[5] = {0, 0, 0, 0, 0};
// const int debounceDelay = 200;
// bool autoMode = true; // Mặc định là chế độ tự động

// // Điều khiển từ Blynk
// BLYNK_WRITE(VPIN_LAMP_OUTDOOR)
// {
//     if (!autoMode)
//         digitalWrite(RELAY_LAMP_OUTDOOR, !param.asInt());
// }
// BLYNK_WRITE(VPIN_LAMP_LIVING)
// {
//     if (!autoMode)
//         digitalWrite(RELAY_LAMP_LIVING, !param.asInt());
// }
// BLYNK_WRITE(VPIN_LAMP3)
// {
//     if (!autoMode)
//         digitalWrite(RELAY_LAMP3, !param.asInt());
// }
// BLYNK_WRITE(VPIN_FAN)
// {
//     if (!autoMode)
//         digitalWrite(RELAY_FAN, !param.asInt());
// }
// BLYNK_WRITE(VPIN_DOOR)
// { // Mở cửa từ Blynk
//     if (param.asInt() == 1)
//     {
//         doorServo.write(90); // Mở cửa
//         Serial.println("Door unlocked via Blynk!");
//         delay(2000);
//         doorServo.write(0); // Đóng lại
//         wrongCardCount = 0;
//     }
// }
// BLYNK_WRITE(VPIN_MODE)
// { // Chuyển chế độ từ Blynk
//     autoMode = param.asInt();
//     Serial.println(autoMode ? "Auto Mode" : "Manual Mode");
// }

// void setup()
// {
//     Serial.begin(115200);

//     // Cấu hình chân
//     pinMode(RELAY_LAMP_OUTDOOR, OUTPUT);
//     pinMode(RELAY_LAMP_LIVING, OUTPUT);
//     pinMode(RELAY_LAMP3, OUTPUT);
//     pinMode(RELAY_FAN, OUTPUT);
//     pinMode(BUZZER, OUTPUT);
//     pinMode(PIR_OUTDOOR, INPUT);
//     pinMode(PIR_LIVING, INPUT);
//     pinMode(LDR_PIN, INPUT);
//     pinMode(BUTTON_MODE, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP_OUTDOOR, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP_LIVING, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP3, INPUT_PULLUP);
//     pinMode(BUTTON_FAN, INPUT_PULLUP);

//     digitalWrite(RELAY_LAMP_OUTDOOR, HIGH); // Tắt relay
//     digitalWrite(RELAY_LAMP_LIVING, HIGH);
//     digitalWrite(RELAY_LAMP3, HIGH);
//     digitalWrite(RELAY_FAN, HIGH);
//     digitalWrite(BUZZER, LOW);

//     dht.begin();
//     SPI.begin(14, 12, 23, 5); // SCK, MISO, MOSI, SS
//     rfid.PCD_Init();
//     doorServo.attach(SERVO_PIN);
//     doorServo.write(0); // Khóa cửa ban đầu

//     Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
// }

// void loop()
// {
//     Blynk.run();

//     // Chuyển chế độ bằng công tắc vật lý
//     if (digitalRead(BUTTON_MODE) == LOW && (millis() - lastDebounce[0] > debounceDelay))
//     {
//         autoMode = !autoMode;
//         Blynk.virtualWrite(VPIN_MODE, autoMode);
//         Serial.println(autoMode ? "Auto Mode" : "Manual Mode");
//         lastDebounce[0] = millis();
//     }

//     // Cập nhật dữ liệu mỗi 2 giây
//     if (millis() - lastUpdate > 2000)
//     {
//         float temp = dht.readTemperature();
//         float humi = dht.readHumidity();
//         int ldrValue = analogRead(LDR_PIN);
//         int pirOutdoor = digitalRead(PIR_OUTDOOR);
//         int pirLiving = digitalRead(PIR_LIVING);

//         // Gửi dữ liệu lên Blynk
//         Blynk.virtualWrite(VPIN_TEMP, temp);
//         Blynk.virtualWrite(VPIN_HUMI, humi);
//         Blynk.virtualWrite(VPIN_LAMP_OUTDOOR, !digitalRead(RELAY_LAMP_OUTDOOR));
//         Blynk.virtualWrite(VPIN_LAMP_LIVING, !digitalRead(RELAY_LAMP_LIVING));
//         Blynk.virtualWrite(VPIN_LAMP3, !digitalRead(RELAY_LAMP3));
//         Blynk.virtualWrite(VPIN_FAN, !digitalRead(RELAY_FAN));

//         // Chế độ tự động
//         if (autoMode)
//         {
//             // Quạt: Nhiệt độ > 30°C
//             if (temp > 30)
//             {
//                 digitalWrite(RELAY_FAN, LOW); // Bật quạt
//             }
//             else
//             {
//                 digitalWrite(RELAY_FAN, HIGH); // Tắt quạt
//             }

//             // Đèn ngoài sân: Trời tối + có người
//             if (ldrValue < 1000 && pirOutdoor == HIGH)
//             {
//                 digitalWrite(RELAY_LAMP_OUTDOOR, LOW); // Bật đèn
//             }
//             else
//             {
//                 digitalWrite(RELAY_LAMP_OUTDOOR, HIGH); // Tắt đèn
//             }

//             // Đèn phòng khách: Trời tối + có người
//             if (ldrValue < 1000 && pirLiving == HIGH)
//             {
//                 digitalWrite(RELAY_LAMP_LIVING, LOW); // Bật đèn
//             }
//             else
//             {
//                 digitalWrite(RELAY_LAMP_LIVING, HIGH); // Tắt đèn
//             }
//         }

//         lastUpdate = millis();
//     }

//     // Chế độ điều khiển tay
//     if (!autoMode)
//     {
//         if (digitalRead(BUTTON_LAMP_OUTDOOR) == LOW && (millis() - lastDebounce[1] > debounceDelay))
//         {
//             digitalWrite(RELAY_LAMP_OUTDOOR, !digitalRead(RELAY_LAMP_OUTDOOR));
//             Blynk.virtualWrite(VPIN_LAMP_OUTDOOR, !digitalRead(RELAY_LAMP_OUTDOOR));
//             Serial.print("Lamp Outdoor: ");
//             Serial.println(digitalRead(RELAY_LAMP_OUTDOOR) ? "OFF" : "ON");
//             lastDebounce[1] = millis();
//         }

//         if (digitalRead(BUTTON_LAMP_LIVING) == LOW && (millis() - lastDebounce[2] > debounceDelay))
//         {
//             digitalWrite(RELAY_LAMP_LIVING, !digitalRead(RELAY_LAMP_LIVING));
//             Blynk.virtualWrite(VPIN_LAMP_LIVING, !digitalRead(RELAY_LAMP_LIVING));
//             Serial.print("Lamp Living: ");
//             Serial.println(digitalRead(RELAY_LAMP_LIVING) ? "OFF" : "ON");
//             lastDebounce[2] = millis();
//         }

//         if (digitalRead(BUTTON_LAMP3) == LOW && (millis() - lastDebounce[3] > debounceDelay))
//         {
//             digitalWrite(RELAY_LAMP3, !digitalRead(RELAY_LAMP3));
//             Blynk.virtualWrite(VPIN_LAMP3, !digitalRead(RELAY_LAMP3));
//             Serial.print("Lamp 3: ");
//             Serial.println(digitalRead(RELAY_LAMP3) ? "OFF" : "ON");
//             lastDebounce[3] = millis();
//         }

//         if (digitalRead(BUTTON_FAN) == LOW && (millis() - lastDebounce[4] > debounceDelay))
//         {
//             digitalWrite(RELAY_FAN, !digitalRead(RELAY_FAN));
//             Blynk.virtualWrite(VPIN_FAN, !digitalRead(RELAY_FAN));
//             Serial.print("Fan: ");
//             Serial.println(digitalRead(RELAY_FAN) ? "OFF" : "ON");
//             lastDebounce[4] = millis();
//         }
//     }

//     // RFID
//     if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
//     {
//         String cardUID = "";
//         for (byte i = 0; i < rfid.uid.size; i++)
//         {
//             cardUID += String(rfid.uid.uidByte[i], HEX);
//         }
//         if (cardUID == validCard)
//         {
//             doorServo.write(90); // Mở cửa
//             Serial.println("Door unlocked!");
//             delay(2000);
//             doorServo.write(0); // Đóng cửa
//             wrongCardCount = 0;
//         }
//         else
//         {
//             wrongCardCount++;
//             if (wrongCardCount >= 3)
//             {
//                 digitalWrite(BUZZER, HIGH);
//                 Blynk.logEvent("wrong_card", "Sai thẻ 3 lần!");
//                 delay(5000);
//                 digitalWrite(BUZZER, LOW);
//                 wrongCardCount = 0;
//             }
//         }
//         rfid.PICC_HaltA();
//     }
// }