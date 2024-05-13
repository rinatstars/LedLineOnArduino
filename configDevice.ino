#define RESPONSE_TIMEOUT 10000

void configureWiFi()
{
  //EspSerial.println(F("AT+RST"));
  //delay(1000);
  
  // Send AT commands to set ESP8266 in AP mode
  EspSerial.println(F("AT+CWMODE=2")); // Set ESP8266 in AP mode
  delay(1000);

  EspSerial.println(F("AT+CWSAP=\"LedLine\",\"12345678\",1,3")); // Set AP name and password
  delay(1000);

  EspSerial.println(F("AT+CIPMUX=1")); // Enable multiple connections
  delay(1000);

  EspSerial.println(F("AT+CIPSERVER=1,80")); // Start server on port 80
  delay(1000);

  // Wait for configuration
  while (!digitalRead(configButtonPin))
  {
    delay(100);
    blinkLED(100);
  }

  // Wait for configuration to be completed
  while (!configurationCompleted)
  {
    if (EspSerial.available()) {
      digitalWrite(ledPin, HIGH);
      processATResponse();
    }
    blinkLED(300);
  }

  EEPROM.put(0, data);
  dataLast = data;
  
    // Send AT command to reboot ESP8266
  EspSerial.println(F("AT+RST"));
  delay(2000);  // Delay to allow ESP8266 to reboot

  // Blink the built-in LED to indicate configuration completion on Arduino
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Reboot Arduino
  asm volatile ("  jmp 0");
}

void processATResponse() {
  while (EspSerial.available()) {
    String response = EspSerial.readStringUntil('\n');
    
    // Поиск строки "GET / HTTP/1.1"
    if (response.indexOf(F("GET /?auth=")) != -1 && response.indexOf(F("&ssid=")) != -1 && response.indexOf(F("&pass=")) != -1 && response.indexOf(F("&serveraddress=")) != -1) {
      // Действие с полученными данными
      parseAndSaveData(response);
    }
  }
}

void parseAndSaveData(String dataWifi) {
  // Разбиваем строку на пары "ключ=значение"
  int startPos = dataWifi.indexOf('?') + 1;
  int endPos = dataWifi.indexOf(' ', startPos);
  String params = dataWifi.substring(startPos, endPos);

  int equalPos = params.indexOf('=');
  while (equalPos != -1) {
    int ampersandPos = params.indexOf('&');

    unsigned long timeStart = millis();
    
    String key, value;
    if (ampersandPos != -1) {
      key = params.substring(0, equalPos);
      value = params.substring(equalPos + 1, ampersandPos);
      params = params.substring(ampersandPos + 1);
    } else {
      key = params.substring(0, equalPos);
      value = params.substring(equalPos + 1);
      params = "";  // Обнуляем params, чтобы выйти из цикла
    }

    // Сохраняем значение в соответствующую переменную
    if (key == "auth") {
      strcpy(data.auth, value.c_str());
    } else if (key == "ssid") {
      strcpy(data.ssid, value.c_str());
    } else if (key == "pass") {
      strcpy(data.pass, value.c_str());
    } else if (key == "serveraddress") {
      strcpy(data.serveraddress, value.c_str());
    }

    equalPos = params.indexOf('=');
    if ((millis() - timeStart) > 10000) {
      Serial.println("TimeOut");
      break;
    }
  }

  // Отправляем обратный ответ
  String acknowledgment = "<html>\r\n"; //HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n
  acknowledgment += "<h1>Auth: " + String(data.auth) + "</h1>\r\n";
  acknowledgment += "<h1>SSID: " + String(data.ssid) + "</h1>\r\n";
  acknowledgment += "<h1>Pass: " + String(data.pass) + "</h1>\r\n";
  acknowledgment += "<h1>ServAd: " + String(data.serveraddress) + "</h1>\r\n";
  acknowledgment += "</html>\n";
  // Отправляем подтверждение ответа от ESP8266
//  EspSerial.println(String(F("AT+CIPSEND=0,")) + String(acknowledgment.length()));
//  delay(500);
//  EspSerial.print(acknowledgment);
//  delay(1000);
//  EspSerial.print(F("AT+CIPCLOSE=0"));
//  delay(1000);

  bool responseReceived = false;
  unsigned long startTime = millis(); // Запоминаем время начала ожидания ответа

  // Проверяем, получен ли ответ от ESP8266
  unsigned long startTimeWebConf = millis();
  while (!responseReceived || (millis() - startTimeWebConf) < RESPONSE_TIMEOUT) {
    EspSerial.println(String(F("AT+CIPSEND=0,")) + String(acknowledgment.length()));
    delay(500);
    EspSerial.print(acknowledgment);
    delay(1000);

    
    while (EspSerial.available() || (millis() - startTimeWebConf) < RESPONSE_TIMEOUT) {
      // Принимаем ответ от ESP8266
      String response = EspSerial.readStringUntil('\n');
      if (response.indexOf("SEND OK") != -1) {
        // Ответ успешно отправлен
        responseReceived = true;
      }
    }
  }

  EspSerial.print(F("AT+CIPCLOSE=0"));
  delay(1000);
  
  configurationCompleted = 1;
  delay(5000);
}
