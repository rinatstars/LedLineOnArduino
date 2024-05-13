//#define BLYNK_PRINT Serial

#define BLYNK_NO_FANCY_LOGO
#define BLYNK_PRINT_DISABLE
#define BLYNK_NO_DEFAULT_BANNER
#define BLYNK_NO_INFO

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include "DHT.h"
#include <EEPROM.h>

WidgetTerminal terminal(V21);

#define DHTPIN 7     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
#define DHTPIN1 2     // what digital pin we're connected to
#define DHTPIN2 8

DHT dht(DHTPIN, DHTTYPE); //кухня
DHT dht1(DHTPIN1, DHTTYPE); //ванная
DHT dht2(DHTPIN2, DHTTYPE); //спальная

uint16_t h = 0;
uint16_t t = 0;
uint16_t h1 = 0;
uint16_t t1 = 0;
uint16_t h2 = 0;
uint16_t t2 = 0;

bool  changeDHT = 0;

#define timeLed 10

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

#define lightPin1 3 // Пин, на который подключено реле
bool  powerOn1 = 0; //включено или выключено устройство
// int lightLed1 = 0; //яркость Led в 100%
uint8_t lastLightLed1 = 0; //яркость Led прошлая в 100%

#define lightPin2 11 // Пин, на который подключено реле
boolean powerOn2 = 0; //включено или выключено устройство
// int lightLed2 = 0; //яркость Led в 100%
uint8_t lastLightLed2 = 0; //яркость Led прошлая в 100%

#define sensLight A7 //датчик света
uint16_t lightHome = 0;
// boolean autoLight = 0; //Включен ли автосвет?
boolean timerAutoLight = 0; //Сработал таймер автосвета
// int porogAutoLight = 800; //Порог срабатывания автосвета

boolean inHome = 0; //Я дома

#define sensMove 4 //датчик движения
boolean moveDetect = 0; // обнаружено движение
boolean moveLight1 = 0; // включить свет по движению 1
uint8_t lightLed1Move1 = 0; // прошлое значение света до включения 1
boolean moveLight2 = 0; // включить свет по движению 2
uint8_t lightLed2Move2 = 0; // прошлое значение света до включения 2
// boolean autoLightMove = 0; // автосвет по движению
// unsigned long lightMoveTime; // когда сработал датчик движения


#define configButtonPin 12  // Пин для кнопки
boolean configurationCompleted = 0; // завершение настройки Wi-Fi и параметров сервера Blynk
const uint8_t ledPin = 13;  // Пин для светодиода на Arduino Nano
const int blinkInterval = 1000;  // Интервал мигания светодиода в миллисекундах

unsigned long previousMillis = 0;  // Переменная для отслеживания времени мигания
unsigned long previousReconnectMillis = 0; // Предыдущее время реконнекта (в миллисекундах)
const unsigned long reconnectInterval = 10000L;  // Интервал реконнекта (в миллисекундах)

int timerID;

unsigned long ledTime1;
unsigned long ledTime2;
unsigned long dhtTime;
unsigned long lightTime;


boolean changeLight1 = 0;
boolean changeLight2 = 0;
boolean printProcess = 0;

//данные для сохранения
struct object {
  // Подсветка 1
  uint8_t lightLed1; //яркость Led в 100%
  
  // Подсветка 2
  uint8_t lightLed2; //яркость Led в 100%
  
  //автосвет
  boolean autoLight; //Включен ли автосвет?
  uint16_t porogAutoLight; //Порог срабатывания автосвета
  
  //датчик движения
  boolean autoLightMove; // автосвет по движению
  uint16_t porogAutoLightMove; //Порог срабатывания автосвета по движению
  boolean autoLightMoveAll; // Включать весь свет по движению или только на кухне

  char auth[33];           // Токен Blynk
  char ssid[16];           // Название точки доступа
  char pass[11];           // Пароль от точки доступа
  char serveraddress[17];  // Адрес сервера

  bool operator!=(const object& a) 
  {
      return (lightLed1 != a.lightLed1 || lightLed2 != a.lightLed2 || autoLight != a.autoLight || porogAutoLight != a.porogAutoLight || autoLightMove != a.autoLightMove || porogAutoLightMove != a.porogAutoLightMove || strcmp(auth, a.auth) != 0 || strcmp(ssid, a.ssid) != 0 || strcmp(pass, a.pass) != 0 || strcmp(serveraddress, a.serveraddress) != 0);
  }

  uint8_t lightLedMoveLevel; // Уровень яркости при движении в 100%
};

object data = {
  // Подсветка 1
  0, // яркость Led в 100%

  // Подсветка 2
  0, // яркость Led в 100%

  // Автосвет
  0, // Включен ли автосвет?
  500, // Порог срабатывания автосвета

  // Датчик движения
  0, // Автосвет по движению
  100, // Порог срабатывания автосвета по движению
  0, // Включать весь свет по движению или только на кухне

  // Настройки сети
  "y4ThCDs8xDYgNzBWlXFS865WhY7qdkqM", // Токен Blynk
  "RT-GPON-C544", // Название точки доступа
  "bBsUG3G4", // Пароль от точки доступа
  "192.168.0.9", // Адрес сервера

  5 // Уровень яркости при движении в 100%
};

object dataLast = {
  // Подсветка 1
  0, //яркость Led в 100%
  
  // Подсветка 2
  0, //яркость Led в 100%
  
  //автосвет
  0, //Включен ли автосвет?
  500, //Порог срабатывания автосвета
  
  //датчик движения
  0, // автосвет по движению
  100, //Порог срабатывания автосвета по движению
  0, // Включать весь свет по движению или только на кухне

  // Настройки сети
  "y4ThCDs8xDYgNzBWlXFS865WhY7qdkqM", // Токен Blynk
  "RT-GPON-C544", // Название точки доступа
  "bBsUG3G4", // Пароль от точки доступа
  "45.85.93.75", // Адрес сервера

  5 // Уровень яркости при движении в 100%
}; 

void setup()
{
  // Debug console
  //Serial.begin(115200);
  pinMode(lightPin1, OUTPUT); // Настройка порта lightPin1 на выход
  pinMode(lightPin2, OUTPUT); // Настройка порта lightPin1 на выход
  pinMode(ledPin, OUTPUT);
  pinMode(sensLight, INPUT);
  pinMode(sensMove, INPUT);
  pinMode(configButtonPin, INPUT_PULLUP);
  
//  EEPROM.put(0, data); // если нужно в первый раз записать EEPROM
  
  EEPROM.get(0, data);//loadData(); //загрузка данных, закоментить если выше раскоментено
  dataLast = data;
  
  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  // Check if Wi-Fi settings are already configured
  if (digitalRead(configButtonPin) == LOW) {
    // If not configured, configure Wi-Fi using AT commands
    digitalWrite(ledPin, HIGH);
    configureWiFi();
  } else {
    // If already configured, connect to Blynk
    Blynk.begin(data.auth, wifi, data.ssid, data.pass, data.serveraddress, 8080);
  }  

  //lightLed1 = 100;
  lastLightLed1 = 0;

  //lightLed2 = 100;
  lastLightLed2 = 0;

  dht.begin();
  dht1.begin();
  dht2.begin();

  previousMillis = millis();
}

void loop()
{
  Blynk.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!

  if (Blynk.connected()) {
    // Если подключено к серверу Blynk, выключаем светодиод
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }

  //датчик освещения
  if (millis() - lightTime > 3000)
  {
    lightHome = analogReadSens(sensLight) / 64;
    lightTime = millis();
    Blynk.virtualWrite(V12, lightHome);

    // Сохраняем данные в EEPROM при изменении
    if (data != dataLast)
      {
        EEPROM.put(0, data);
        dataLast = data;
      }
  }

  // Датчик движения
  if (digitalRead(sensMove) && data.autoLightMove && (data.porogAutoLightMove > (analogReadSens(sensLight) / 64))) //автосвет по движению
  {
    if (!powerOn1 & data.autoLightMoveAll)
    {
      moveLight1 = 1;
      lightLed1Move1 = data.lightLed1;
      Blynk.virtualWrite(V1, data.lightLedMoveLevel);
      Blynk.virtualWrite(V0, 1);
      data.lightLed1 = data.lightLedMoveLevel;
      powerOn1 = 1;
      changeLight1 = 1;
    }
    
    if (!powerOn2)
    {
      moveLight2 = 1;
      lightLed2Move2 = data.lightLed2;
      Blynk.virtualWrite(V3, data.lightLedMoveLevel);
      Blynk.virtualWrite(V2, 1);
      data.lightLed2 = data.lightLedMoveLevel;
      powerOn2 = 1;
      changeLight2 = 1;
    }
  } else
  {
    // Выключение света при отсутствии движения
    if (moveLight1)
    {
      moveLight1 = 0;
      Blynk.virtualWrite(V1, lightLed1Move1);
      Blynk.virtualWrite(V0, 0);
      powerOn1 = 0; 
      changeLight1 = 1;
      data.lightLed1 = lightLed1Move1;
    }
    if (moveLight2)
    {
      moveLight2 = 0;
      Blynk.virtualWrite(V3, lightLed2Move2);
      Blynk.virtualWrite(V2, 0);
      powerOn2 = 0; 
      changeLight2 = 1;
      data.lightLed2 = lightLed2Move2;
    }
  }

  //включаем плавно свет №1
  if (changeLight1 && (millis() - ledTime1 > 30))
  {
    boolean al = (data.autoLight) && (timerAutoLight) && (data.porogAutoLight > lightHome) && (inHome); // && (lastLightLed1 != 3);
    if ((lastLightLed1 != data.lightLed1) && (powerOn1))
    {
      if (lastLightLed1 < data.lightLed1) lastLightLed1 = lastLightLed1 + 1;
      else lastLightLed1 = lastLightLed1 - 1;
    } else if ((!powerOn1) && (al) && (lastLightLed1 != 3)) //автосвет
    {
      if (lastLightLed1 > 3) lastLightLed1 = lastLightLed1 - 1;
      else lastLightLed1 = lastLightLed1 + 1;
    } else if ((!powerOn1) && (lastLightLed1 > 0) && !(al)) //выключен свет
    {
      lastLightLed1 = lastLightLed1 - 1;
    } else 
    {
      changeLight1 = 0;
      return;
    }
    setLight( lastLightLed1, lightPin1 );
    ledTime1 = millis();
  }

  //включаем плавно свет №2
  if (changeLight2 && (millis() - ledTime2 > 30))
  {
    boolean al = (data.autoLight) && (timerAutoLight) && (data.porogAutoLight > lightHome) && (inHome); // && (lastLightLed2 != 3);
    if ((lastLightLed2 != data.lightLed2) && (powerOn2))
    {
      if (lastLightLed2 < data.lightLed2) lastLightLed2 = lastLightLed2 + 1;
      else lastLightLed2 = lastLightLed2 - 1;
    } else if ((!powerOn2) && (al) && (lastLightLed2 != 3)) //автосвет
    {
      if (lastLightLed2 > 3) lastLightLed2 = lastLightLed2 - 1;
      else lastLightLed2 = lastLightLed2 + 1;
    } else if ((!powerOn2) && (lastLightLed2 > 0) && !(al)) //выключен свет
    {
      lastLightLed2 = lastLightLed2 - 1;
    } else
    {
      changeLight2 = 0;
      return;
    }
    setLight( lastLightLed2, lightPin2 );
    ledTime2 = millis();
  }

  //опрашиваем датчики температуры/влажности
  if ((millis() - dhtTime > 3000) && !changeLight1 && !changeLight2)
  {
    dhtTime = millis();
    
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    uint16_t newH = (uint16_t)dht.readHumidity()*10;
    // Read temperature as Celsius (the default)
    uint16_t newT = (uint16_t)(dht.readTemperature()*10);
    uint16_t newH1 = (uint16_t)dht1.readHumidity()*10;
    // Read temperature as Celsius (the default)
    uint16_t newT1 = (uint16_t)(dht1.readTemperature()*10);
    uint16_t newH2 = (uint16_t)dht2.readHumidity()*10;
    // Read temperature as Celsius (the default)
    uint16_t newT2 = (uint16_t)(dht2.readTemperature()*10);

    if (h != newH || t != newT || h1 != newH1 || t1 != newT1 || h2 != newH2 || t2 != newT2 )
    {
      h = newH; t = newT; h1 = newH1; t1 = newT1; h2 = newH2; t2 = newT2;

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Blynk.virtualWrite(V4, 0);
      } else
      {
        Blynk.virtualWrite(V4, 255);
  
        // Compute heat index in Celsius (isFahreheit = false)
        uint16_t hic = (uint16_t)dht.computeHeatIndex(t/10.0, h/10, false)*10;
  
        Blynk.virtualWrite(V5, h/10.0);
        Blynk.virtualWrite(V6, t/10.0);
        Blynk.virtualWrite(V7, hic/10.0);
      }
  
      if (isnan(h1) || isnan(t1)) {
        Blynk.virtualWrite(V8, 0);
      } else
      {
        Blynk.virtualWrite(V8, 255);
  
        // Compute heat index in Celsius (isFahreheit = false)
        uint16_t hic1 = (uint16_t)dht1.computeHeatIndex(t1/10.0, h1/10, false)*10;
  
        Blynk.virtualWrite(V9, h1/10.0);
        Blynk.virtualWrite(V10, t1/10.0);
        Blynk.virtualWrite(V11, hic1/10.0);
      }
  
      if (isnan(h2) || isnan(t2)) {
        Blynk.virtualWrite(V13, 0);
      } else
      {
        Blynk.virtualWrite(V13, 255);
  
        // Compute heat index in Celsius (isFahreheit = false)
        uint16_t hic2 = (uint16_t)dht2.computeHeatIndex(t2/10.0, h2/10, false)*10;
  
        Blynk.virtualWrite(V14, h2/10.0);
        Blynk.virtualWrite(V15, t2/10.0);
        Blynk.virtualWrite(V16, hic2/10.0);
      }
    }

    if (printProcess) {
      terminal.println(millis());
      terminal.println(data != dataLast);
      terminal.flush();
    }
  }
}

BLYNK_WRITE(V0) // Вызовется при обновлении виртуального порта V0
{
  powerOn1 = param.asInt(); // Считываем новое значение порта
  changeLight1 = 1;
}

BLYNK_WRITE(V1) // Вызовется при обновлении виртуального порта V0
{
  if (powerOn1) lastLightLed1 = data.lightLed1;
  data.lightLed1 = (uint8_t)param.asInt();
  powerOn1 = 1;
  Blynk.virtualWrite(V0, 1);
  changeLight1 = 1;
}

BLYNK_WRITE(V2) // Вызовется при обновлении виртуального порта V0
{
  powerOn2 = param.asInt(); // Считываем новое значение порта
  changeLight2 = 1;
}

BLYNK_WRITE(V3) // Вызовется при обновлении виртуального порта V0
{
  if (powerOn2) lastLightLed2 = data.lightLed2;
  data.lightLed2 = (uint8_t)param.asInt();
  powerOn2 = 1;
  Blynk.virtualWrite(V2, 1);
  changeLight2 = 1;
}

void setLight( int led, int pin)
{
  if (led != 0 && led != 100)
  {
    int lightLed = int(pow(10, ((float(led) + 70) / 70.2)) - 9);      //map(param.asInt(), 0, 100, 0, 1024); // Считываем новое значение порта
    analogWrite(pin, lightLed); // Устанавливаем значение на пин
  } else if (led == 100) analogWrite(pin, 255);
  else analogWrite(pin, 0);
}

BLYNK_WRITE(V17) // Вызовется при обновлении виртуального порта V0
{
  inHome = param.asInt(); // Считываем новое значение порта
  changeLight1 = 1;
  changeLight2 = 1;
}

BLYNK_WRITE(V18) // Вызовется при обновлении виртуального порта V0
{
  timerAutoLight = param.asInt(); // Считываем новое значение порта
  changeLight1 = 1;
  changeLight2 = 1;
}

BLYNK_WRITE(V19) // Вызовется при обновлении виртуального порта V0
{
  data.porogAutoLight = (uint16_t)param.asInt(); // Считываем новое значение порта
  changeLight1 = 1;
  changeLight2 = 1;
}

BLYNK_WRITE(V20) // Вызовется при обновлении виртуального порта V0
{
  data.autoLight = (uint16_t)param.asInt(); // Считываем новое значение порта
  changeLight1 = 1;
  changeLight2 = 1;
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V17, V18, V19, V20);
}

BLYNK_WRITE(V21) //terminal
{
  if (String("var") == param.asStr()) {
    int VV[] = {powerOn1, data.lightLed1, powerOn2, data.lightLed2, V4, V5, V6, V7, V8, V9, V10, V11, lightHome, V13, V14, V15, V16, inHome, timerAutoLight, data.porogAutoLight, data.autoLight};
    for ( int j = 0 ; j <= 20; j++)
    {
      terminal.print("V");
      terminal.print( j );
      terminal.print(" = ");
      terminal.println(VV[j]);
    }
  } else if (String(F("print_on")) == param.asStr()) {
    printProcess = 1;
  } else if (String(F("print_off")) == param.asStr()) {
    printProcess = 0;
  } else {
    // Send it back
    terminal.print("You said:");
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }

  // Ensure everything is sent
  terminal.flush();
}

BLYNK_WRITE(V22) // Вызовется при обновлении виртуального порта V0
{
  data.autoLightMove = param.asInt(); // Считываем новое значение порта
}

BLYNK_WRITE(V23) // Вызовется при обновлении виртуального порта V0
{
  data.porogAutoLightMove = param.asInt(); // Считываем новое значение порта
}

BLYNK_WRITE(V24) // Вызовется при обновлении виртуального порта V24. Включать свет при движении везде или только на кухе
{
  data.autoLightMoveAll = param.asInt(); // Считываем новое значение порта
}

BLYNK_WRITE(V25) // Вызовется при обновлении виртуального порта V0. Яркость при движении
{
  data.lightLedMoveLevel = (uint8_t)param.asInt();
}

void blinkLED(unsigned int tblinkInterval) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    // Инвертируем состояние светодиода
    digitalWrite(ledPin, !digitalRead(ledPin));
  }
}
