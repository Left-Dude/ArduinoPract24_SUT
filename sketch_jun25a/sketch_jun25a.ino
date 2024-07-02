#include "DHT.h"
#include "TroykaMQ.h"

// #include "Bonezegei_DHT11.h"
#define DHTPIN 2 // номер пина, к которому подсоединен датчик
#define popin 0

// Раскомментируйте в соответствии с используемым датчиком

// Инициируем датчик

DHT dht(DHTPIN, DHT11);

MQ2 mq2(A0);
MQ3 mq3(A0);

int MQ3sensorValue = 0;  // value read from the sensor

void datMQ3() {
  MQ3sensorValue = analogRead(popin);
  Serial.println(MQ3sensorValue);
  // interpretation 
   // Detecting range: 20ppm-2000ppm carbon monoxide 
   // air quality-cases: < 200 perfect, 200 - 800 normal, > 800 - 1800 high, > 1800 abnormal 
  if (MQ3sensorValue <= 200)  
  { 
    Serial.println("Air-Quality: CO perfect"); 
  } 
  else if ((MQ3sensorValue <= 800))
  { 
    Serial.println("Air-Quality: CO normal"); 
  } 
  else if ((MQ3sensorValue <= 1800)) 
  { 
    Serial.println("Air-Quality: CO high"); 
  } 
  else if (MQ3sensorValue > 1800)  
  { 
    Serial.println("Air-Quality: ALARM CO very high");  
  } 
  else 
  { 
    Serial.println("MQ-3 - cant read any value - check the sensor!"); 
  }  
}


void datDHT() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Проверка удачно прошло ли считывание.
  
  if (isnan(h) || isnan(t)) {
  //Serial.println("Не удается считать показания ");
  datIKD();

  return;
  }
  Serial.println("Датчик влажности");
  Serial.print("Влажность: "); Serial.print(h); Serial.print(" %\t"); Serial.print("Температура: "); Serial.print(t); Serial.print(" *C\n");
}

void datIKD(){
  int v;
  v = digitalRead( DHTPIN );
  Serial.println("Данные от Инфракрасный датчик:");
  Serial.println(v); 
}

void dateMQ_Check(){
  // выводим отношения текущего сопротивление датчика
  // к сопротивлению датчика в чистом воздухе (Rs/Ro)
  Serial.print("Ratio: ");
  Serial.print(mq2.readRatio());
  // выводим значения газов в ppm
  Serial.print("LPG: ");
  Serial.print(mq2.readLPG());
  Serial.print(" ppm ");
  Serial.print(" Methane: ");
  Serial.print(mq2.readMethane());
  Serial.print(" ppm ");
  Serial.print(" Smoke: ");
  Serial.print(mq2.readSmoke());
  Serial.print(" ppm ");
  Serial.print(" Hydrogen: ");
  Serial.print(mq2.readHydrogen());
  Serial.println(" ppm ");
  delay(100);
}

void dateMQ3(){
  // выводим отношения текущего сопротивление датчика
  // к сопротивление датчика в чистом воздухе (Rs/Ro)
  Serial.print("Ratio: ");
  Serial.print(mq3.readRatio());
  // выводим значения паров алкоголя
  Serial.print(" Alcohol: ");
  Serial.print(mq3.readAlcoholMgL());
  Serial.print(" mG/L ");
  Serial.print(mq3.readAlcoholPpm());
  Serial.println(" ppm ");
  delay(100);
}

/*
void checkDetector(){
  if (){

  }
  

  float h2 = dht.readHumidity();
  float t2 = dht.readTemperature();
  // Проверка удачно прошло ли считывание.
  if (!(isnan(h) || isnan(t))) {
    datDHT();
  }

  if (){

  }
}*/

void setup() {
pinMode(popin, INPUT);
analogWrite(popin, HIGH);
Serial.begin(9600);
dht.begin();
  // перед калибровкой датчика прогрейте его 60 секунд
  // выполняем калибровку датчика на чистом воздухе
  mq2.calibrate();
  mq3.calibrate();
  // выводим сопротивление датчика в чистом воздухе (Ro) в serial-порт
  Serial.print("Ro = ");
  Serial.println(mq2.getRo());
  Serial.println(mq3.getRo());
}

void loop() {

  //float h = dht.readHumidity();
  //float t = dht.readTemperature();

  // Проверка удачно прошло ли считывание.

  //if (isnan(h) || isnan(t)) {}


// Задержка 2 секунды между измерениями
  //datMQ3();
  dateMQ3();

  delay(500);
}
