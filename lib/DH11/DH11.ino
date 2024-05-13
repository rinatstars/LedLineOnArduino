#include <MsTimer2.h>
#include "DHT.h"
#include <SPI.h>
#include <SD.h>


#define DHTPIN A1     // what pin we're connected to
#define sensLight A3  //датчик света
#define sensTemp A2  //датчик температуры 
#define fan 9  //датчик температуры o
//#define volt A4  //напряжение управления вентилятора

#define TIME_OUT  50   // время таймаута приема команды (мс)

const int chipSelect = 4; //select SD карты

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

//volatile int rpmFan;

byte dataSerialBuf[11]; // буфер данных для передачи
byte n; // число принятых байтов

int timeCount; //счетчик времени
byte timeOutCount; // счетчик времени тайм-аута пакета

int speedFan = 70;
int speedFan100[] = {0, 29.44, 37.49, 52.73, 66.32, 78.01,
                      87.88, 94.37, 98.27, 99.57, 100};
int speedFan255 = 0;

float temp = 0;
int tempResist = 9850;
int tempR0 = 10000;
int temp0 = 25;
int tempB = 3950;

float h = 0;
float t = 0;
float l = 0;

int sdPrint = 1;  //количество печатей
int firstDataLog = 1; //запись на карту сразу при запуске

String timeStart = "";
String dataStart = "";

//void rpm(){
//  rpmFan++;
//}

//формат передачи данных
// * byte адресс контроллера + код операции 0x10
// 0...3 float температура
// 4...7 float влажность
// 8...9 int   свет
// 10    byte  резерв
// 11,12 int контрольная сумма

//формат приема данных
// 0  byte  адресс контроллера + код операции 0x10
// 1  byte  скорость вентилятора 0...100%
// 2  byte  контрольный код (байт 0 ^ байт 1 ^ 0xe5)

//код операции
// 0x5 

void setup() {
  //TCCR1B |= 1<<CS10;
  //TCCR1B &= ~((1<<CS12)|(1<<CS11));
  
  pinMode(fan, OUTPUT);
  pinMode(sensLight, INPUT);
  pinMode(sensTemp, INPUT);
  //pinMode(volt, INPUT);
  Serial.begin(9600);
  //Serial.println("DHT test");

  dht.begin();
  
  //attachInterrupt(0, rpm, CHANGE);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    //return;
  }else Serial.println("card initialized.");

  MsTimer2::set(1, timerInterrupt); //прерывания по таймеру 1 мс
  MsTimer2::start(); // разрешаем прерывания по таймеру
}

void loop() {  
  //rpmFan = 0;

  //------------------------------- измерение температуры
  if (timeCount < 50) {
    timeCount= 50;

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    h = dht.readHumidity();
    //float t = dht.readTemperature();
    l = analogRead(sensLight);

    //температура ****************************************************************
    temp = tempCalculate(analogRead(sensTemp));
  }

  if (timeCount > 2050) {
    timeCount = 0;
    
    noInterrupts();
    
    * (float *)(dataSerialBuf) = temp;
    * (float *)(dataSerialBuf+4) = h;
    * (int *) (dataSerialBuf+8) = l;
    dataSerialBuf[10]= 0;
    //for (int i=0;i<10;i++){
    //  Serial.println(dataSerialBuf[i]);
    //}
    interrupts();

    if (h < 45) {
      if (analogRead(sensLight) < 80 && speedFan > 51) {
        speedFan255 = map(20, 0, 100, 0, 254);
        
        //speedFan255 = speedFan255 +
        //  0.2490234*(204.8-analogRead(volt));
        analogWrite(fan, speedFan255);
      } else {
          //for (int i=0; i<10; i++) {
            //if (speedFan100[i] <= speedFan && speedFan100[i+1] >= speedFan) {
              speedFan255 = map(speedFan, 0, 100,
                                  0, 254);
           // }
         // }
        
  //      speedFan255 = speedFan255 +
  //      0.2490234*(speedFan*10.24-analogRead(volt));
        analogWrite(fan, speedFan255);
      }
    } else {
      analogWrite(fan, 0);
    }
  }

  if (millis()/600000 == sdPrint || firstDataLog == 1)  //1800000
  { 
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) {
      Serial.println("n\r\n");
    } else {
      
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      
      if (dataFile) {
        String dataString = "";
        if (sdPrint == 1) {
          dataString += "№";
          dataString += '\t';
          dataString += "Температура";
          dataString += '\t';
          dataString += "Влажность";
          dataString += '\t';
          dataString += "Свет";
          dataFile.println(dataString);
          dataString = "";
        }
        dataString += String(sdPrint);
        dataString += '\t';
        dataString += String(temp);
        dataString += '\t';
        dataString += String(h);
        dataString += '\t';
        dataString += String(l);
        dataFile.println(dataString);
        dataFile.close();
        sdPrint++;
        firstDataLog = 0;
      }
      // if the file isn't open, pop up an error:
      else {
        // Serial.println("error opening datalog.txt");
      }
    }
  }
  
  //Serial.println(analogRead(volt));

  /*
  

  serialEvent();

  if (h < 45) {
    if (analogRead(sensLight) < 80 && speedFan > 51) {
      speedFan255 = map(20, 0, 100, 0, 254);
      
      //speedFan255 = speedFan255 +
      //  0.2490234*(204.8-analogRead(volt));
      analogWrite(fan, speedFan255);
    } else {
        //for (int i=0; i<10; i++) {
          //if (speedFan100[i] <= speedFan && speedFan100[i+1] >= speedFan) {
            speedFan255 = map(speedFan, 0, 100,
                                0, 254);
         // }
       // }
      
//      speedFan255 = speedFan255 +
//      0.2490234*(speedFan*10.24-analogRead(volt));
      analogWrite(fan, speedFan255);
    }
  } else {
    analogWrite(fan, 0);
  }

//  Serial.println(speedFan);
//  Serial.println(speedFan255/2.55);
//  Serial.println(analogRead(volt)/10.24);
//  Serial.println("n\r\n");
*/
}

void timerInterrupt() {
  timeCount++;

  timeOutCount++;
  n = Serial.available(); // число принятых байтов
  //if (Serial.available() > 0) Serial.println(Serial.available());

  if(n == 0) timeOutCount = 0; // нет данных

  else if (n == 3){
    byte buf[3];
    // чтение команды из бувера
    buf[0] = Serial.read();
    buf[1] = Serial.read();
    buf[2] = Serial.read();
    // проверка

//    Serial.write(buf[0]);
//    Serial.write(buf[1]);
//    Serial.write(buf[2]);
    
    if ((buf[0] == 0x10) && ((buf[0] ^ buf[1] ^ 0xe5) == buf[2]) ) {
      speedFan = buf[1];
      
      // ответ если значения не NaN
      if (isnan(t) || isnan(h)) {
      } else {
        unsigned int sum= 0; // контрольная сумма
        for (int i=0; i<11; i++) {
          Serial.write(dataSerialBuf[i]);
          sum += dataSerialBuf[i];
        }
        // контрольная сумма ответа
        sum ^= 0xa1e3;
        Serial.write( * ((byte *)(& sum)));
        Serial.write( * (((byte *)(& sum)) + 1)); 
      }
    } 
    else {
      // неправильно, сброс порта
      timeOutCount= 0;
      while (true) { if (Serial.read() == 0xffff) break;} 
    }
  }
  else if (n > 3){
    // принято больше данных, неправильно, сброс порта
    timeOutCount= 0;
    while (true) { if (Serial.read() == 0xffff) break;} 
  }
  else {
    // не все байты приняты, проверка тайм-аута
    if (timeOutCount > TIME_OUT) {
    // сброс порта
    timeOutCount= 0;
    while (true) { if (Serial.read() == 0xffff) break;}
    }
  }
}

float tempCalculate(float temp){
  temp = 1023/temp - 1;
  temp = tempResist / temp;
  temp = temp/tempR0;
  temp = log(temp);
  temp /= tempB;
  temp += 1.0/(temp0 + 273.15);
  temp = 1.0/temp;
  temp -= 273.15;
  temp = floor(10*temp)/10.0;
  return temp;
}

/*
void serialEvent() {
  String inString = "";
  while (Serial.available()) {
    // get the new byte:
    inString = Serial.read();
    }
  if (inString.charAt(0) == 't'){
    int i = 0;
    while (inString.charAt(i) != 'd'){
      timeStart = timeStart + inString.charAt(i);
      i++;
    }
    for (i; i < inString.length(); i++){
      dataStart = dataStart + inString.charAt(i);
    }
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
        
    if (dataFile) {
      String dataString = "";
        dataString += "№";
        dataString += '\t';
        dataString += "Температура";
        dataString += '\t';
        dataString += "Влажность";
        dataString += '\t';
        dataString += "Свет";
        dataString += '\t';
        dataString += "Дата";
        dataString += '\t';
        dataString += "Время";
        dataFile.println(dataString);
        dataString = "";
      dataString += String(sdPrint);
      dataString += '\t';
      dataString += String(temp);
      dataString += '\t';
      dataString += String(h);
      dataString += '\t';
      dataString += String(l);
      dataString += '\t';
      dataString += String(dataStart);
      dataString += '\t';
      dataString += String(timeStart);
      dataFile.println(dataString);
      dataFile.close();
      sdPrint++;
      firstDataLog = 0;
    }
  } else {
    speedFan = (int)Serial.read();
  }
  
}

void sendVar(String nameVar, float var[], unsigned int num)  {
  int i = 0;
    Serial.print('y');
    for (int i = 0; i < num; i++) {
      Serial.print(nameVar.charAt(i));
      Serial.print(var[i]);
      Serial.print(';');
    }
    Serial.print("\r\n");
}
*/
