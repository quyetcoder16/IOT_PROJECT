// #include <Arduino.h>
// #include <SPI.h>
// #include <MFRC522.h>
// #include <DHT.h>
// #include <ESP32Servo.h> // Thư viện cho Servo trên ESP32

// // Định nghĩa chân RFID
// #define SS_PIN 5  // SDA
// #define RST_PIN 0 // RST (GPIO0)

// // Định nghĩa chân relay
// #define RELAY_LAMP_OUTDOOR 16 // Đèn ngoài sân
// #define RELAY_LAMP_LIVING 17  // Đèn phòng khách
// #define RELAY_LAMP3 21        // Đèn 3
// #define RELAY_FAN 22          // Quạt

// // Định nghĩa chân công tắc
// #define BUTTON_MODE 32         // Công tắc chế độ
// #define BUTTON_LAMP_OUTDOOR 33 // Công tắc Đèn ngoài sân
// #define BUTTON_LAMP_LIVING 15  // Công tắc Đèn phòng khách
// #define BUTTON_LAMP3 2         // Công tắc Đèn 3
// #define BUTTON_FAN 13          // Công tắc Quạt

// // Định nghĩa chân DHT22
// #define DHTPIN 4
// #define DHTTYPE DHT22

// // Định nghĩa chân LDR, Servo, PIR, Buzzer
// #define LDR_PIN 34     // Cảm biến ánh sáng
// #define SERVO_PIN 12   // Servo
// #define PIR_OUTDOOR 25 // PIR ngoài sân
// #define PIR_LIVING 26  // PIR phòng khách
// #define BUZZER 14      // Còi

// MFRC522 rfid(SS_PIN, RST_PIN);
// MFRC522::MIFARE_Key key;
// byte nuidPICC[4];
// DHT dht(DHTPIN, DHTTYPE);
// Servo servo; // Khai báo servo

// unsigned long lastDebounce[5] = {0, 0, 0, 0, 0}; // Thời gian chống dội công tắc
// const int debounceDelay = 200;                   // Thời gian chống dội (ms)
// unsigned long lastDHTUpdate = 0;                 // Thời gian cập nhật DHT
// const int dhtUpdateInterval = 2000;              // Cập nhật DHT mỗi 2 giây
// unsigned long lastServoMove = 0;                 // Thời gian servo di chuyển
// unsigned long lastBuzzer = 0;                    // Thời gian còi kêu

// void printHex(byte *buffer, byte bufferSize);
// void printDec(byte *buffer, byte bufferSize);

// void setup()
// {
//     Serial.begin(9600);

//     // Khởi động SPI cho RFID
//     SPI.begin(18, 19, 23, 5); // SCK=18, MISO=19, MOSI=23, SS=5
//     rfid.PCD_Init();
//     for (byte i = 0; i < 6; i++)
//     {
//         key.keyByte[i] = 0xFF;
//     }

//     // Khởi động DHT22
//     dht.begin();

//     // Cấu hình chân relay
//     pinMode(RELAY_LAMP_OUTDOOR, OUTPUT);
//     pinMode(RELAY_LAMP_LIVING, OUTPUT);
//     pinMode(RELAY_LAMP3, OUTPUT);
//     pinMode(RELAY_FAN, OUTPUT);
//     digitalWrite(RELAY_LAMP_OUTDOOR, HIGH);
//     digitalWrite(RELAY_LAMP_LIVING, HIGH);
//     digitalWrite(RELAY_LAMP3, HIGH);
//     digitalWrite(RELAY_FAN, HIGH);

//     // Cấu hình chân công tắc
//     pinMode(BUTTON_MODE, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP_OUTDOOR, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP_LIVING, INPUT_PULLUP);
//     pinMode(BUTTON_LAMP3, INPUT_PULLUP);
//     pinMode(BUTTON_FAN, INPUT_PULLUP);

//     // Cấu hình LDR, Servo, PIR, Buzzer
//     pinMode(LDR_PIN, INPUT);
//     servo.attach(SERVO_PIN, 500, 2400); // Gắn chân servo, min 500us, max 2400us
//     servo.write(0);                     // Đặt servo về vị trí ban đầu (đóng)
//     pinMode(PIR_OUTDOOR, INPUT);
//     pinMode(PIR_LIVING, INPUT);
//     pinMode(BUZZER, OUTPUT);
//     digitalWrite(BUZZER, LOW);

//     Serial.println(F("Kiem tra tat ca linh kien - San sang!"));
//     Serial.println(F("Quet RFID, nhan cong tac, hoac doi cac cam bien..."));
//     Serial.print(F("Using the following key:"));
//     printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
//     Serial.println();
// }

// void loop()
// {
//     // Kiểm tra RFID
//     if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
//     {
//         Serial.print(F("PICC type: "));
//         MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
//         Serial.println(rfid.PICC_GetTypeName(piccType));
//         if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
//             piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
//             piccType != MFRC522::PICC_TYPE_MIFARE_4K)
//         {
//             Serial.println(F("Your tag is not of type MIFARE Classic."));
//         }
//         else if (rfid.uid.uidByte[0] != nuidPICC[0] ||
//                  rfid.uid.uidByte[1] != nuidPICC[1] ||
//                  rfid.uid.uidByte[2] != nuidPICC[2] ||
//                  rfid.uid.uidByte[3] != nuidPICC[3])
//         {
//             Serial.println(F("A new card has been detected."));
//             for (byte i = 0; i < 4; i++)
//             {
//                 nuidPICC[i] = rfid.uid.uidByte[i];
//             }
//             Serial.println(F("The NUID tag is:"));
//             Serial.print(F("In hex: "));
//             printHex(rfid.uid.uidByte, rfid.uid.size);
//             Serial.println();
//             Serial.print(F("In dec: "));
//             printDec(rfid.uid.uidByte, rfid.uid.size);
//             Serial.println();
//             // Mở servo khi quẹt thẻ
//             servo.write(90);
//             Serial.println("Servo: OPEN");
//             lastServoMove = millis();
//         }
//         else
//         {
//             Serial.println(F("Card read previously."));
//         }
//         rfid.PICC_HaltA();
//         rfid.PCD_StopCrypto1();
//     }

//     // Đóng servo sau 2 giây
//     if (lastServoMove > 0 && millis() - lastServoMove > 2000)
//     {
//         servo.write(0);
//         Serial.println("Servo: CLOSED");
//         lastServoMove = 0;
//     }

//     // Kiểm tra công tắc chế độ
//     if (digitalRead(BUTTON_MODE) == LOW && (millis() - lastDebounce[0] > debounceDelay))
//     {
//         bool currentState = digitalRead(RELAY_LAMP_OUTDOOR);
//         digitalWrite(RELAY_LAMP_OUTDOOR, !currentState);
//         digitalWrite(RELAY_LAMP_LIVING, !currentState);
//         digitalWrite(RELAY_LAMP3, !currentState);
//         digitalWrite(RELAY_FAN, !currentState);
//         Serial.print("Cong tac Che do: NHAN - Tat ca relay: ");
//         Serial.println(!currentState ? "ON" : "OFF");
//         lastDebounce[0] = millis();
//     }

//     // Công tắc Đèn ngoài sân
//     if (digitalRead(BUTTON_LAMP_OUTDOOR) == LOW && (millis() - lastDebounce[1] > debounceDelay))
//     {
//         digitalWrite(RELAY_LAMP_OUTDOOR, !digitalRead(RELAY_LAMP_OUTDOOR));
//         Serial.print("Cong tac Den ngoai san: NHAN - Relay: ");
//         Serial.println(digitalRead(RELAY_LAMP_OUTDOOR) ? "OFF" : "ON");
//         lastDebounce[1] = millis();
//     }

//     // Công tắc Đèn phòng khách
//     if (digitalRead(BUTTON_LAMP_LIVING) == LOW && (millis() - lastDebounce[2] > debounceDelay))
//     {
//         digitalWrite(RELAY_LAMP_LIVING, !digitalRead(RELAY_LAMP_LIVING));
//         Serial.print("Cong tac Den phong khach: NHAN - Relay: ");
//         Serial.println(digitalRead(RELAY_LAMP_LIVING) ? "OFF" : "ON");
//         lastDebounce[2] = millis();
//     }

//     // Công tắc Đèn 3
//     if (digitalRead(BUTTON_LAMP3) == LOW && (millis() - lastDebounce[3] > debounceDelay))
//     {
//         digitalWrite(RELAY_LAMP3, !digitalRead(RELAY_LAMP3));
//         Serial.print("Cong tac Den 3: NHAN - Relay: ");
//         Serial.println(digitalRead(RELAY_LAMP3) ? "OFF" : "ON");
//         lastDebounce[3] = millis();
//     }

//     // Công tắc Quạt
//     if (digitalRead(BUTTON_FAN) == LOW && (millis() - lastDebounce[4] > debounceDelay))
//     {
//         digitalWrite(RELAY_FAN, !digitalRead(RELAY_FAN));
//         Serial.print("Cong tac Quat: NHAN - Relay: ");
//         Serial.println(digitalRead(RELAY_FAN) ? "OFF" : "ON");
//         lastDebounce[4] = millis();
//     }

//     // Kiểm tra DHT22 và LDR mỗi 2 giây
//     if (millis() - lastDHTUpdate > dhtUpdateInterval)
//     {
//         float temp = dht.readTemperature();
//         float humi = dht.readHumidity();
//         int ldrValue = analogRead(LDR_PIN);

//         if (isnan(temp) || isnan(humi))
//         {
//             Serial.println("Failed to read from DHT22 sensor!");
//         }
//         else
//         {
//             Serial.print("Temperature: ");
//             Serial.print(temp);
//             Serial.print(" °C    Humidity: ");
//             Serial.print(humi);
//             Serial.print(" %    Light: ");
//             Serial.print(ldrValue);
//             Serial.println(" (0-4095)");
//         }
//         lastDHTUpdate = millis();
//     }

//     // Kiểm tra PIR
//     if (digitalRead(PIR_OUTDOOR) == HIGH)
//     {
//         Serial.println("PIR Ngoai san: CO CHUYEN DONG");
//         digitalWrite(BUZZER, HIGH);
//         lastBuzzer = millis();
//     }
//     if (digitalRead(PIR_LIVING) == HIGH)
//     {
//         Serial.println("PIR Phong khach: CO CHUYEN DONG");
//         digitalWrite(BUZZER, HIGH);
//         lastBuzzer = millis();
//     }

//     // Tắt còi sau 1 giây
//     if (lastBuzzer > 0 && millis() - lastBuzzer > 1000)
//     {
//         digitalWrite(BUZZER, LOW);
//         lastBuzzer = 0;
//     }
// }

// void printHex(byte *buffer, byte bufferSize)
// {
//     for (byte i = 0; i < bufferSize; i++)
//     {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], HEX);
//     }
// }

// void printDec(byte *buffer, byte bufferSize)
// {
//     for (byte i = 0; i < bufferSize; i++)
//     {
//         Serial.print(buffer[i] < 0x10 ? " 0" : " ");
//         Serial.print(buffer[i], DEC);
//     }
// }