#include "DHT.h"
#include "TroykaMQ.h"

// #include "Bonezegei_DHT11.h"
#define DHTPIN 2 // номер пина, к которому подсоединен датчик
#define popin 0

// Раскомментируйте в соответствии с используемым датчиком

// Инициируем датчик

DHT dht(DHTPIN, DHT11);

int MQ3sensorValue = 0;   // value read from the sensor 

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
  else if ((MQ3sensorValue > 200) || (MQ3sensorValue <= 800)) // || = or 
  { 
    Serial.println("Air-Quality: CO normal"); 
  } 
  else if ((MQ3sensorValue > 800) || (MQ3sensorValue <= 1800)) 
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

}

void loop() {

  //float h = dht.readHumidity();
  //float t = dht.readTemperature();

  // Проверка удачно прошло ли считывание.

  //if (isnan(h) || isnan(t)) {}


// Задержка 2 секунды между измерениями
  //datMQ3();
  datIKD();

  delay(500);
}