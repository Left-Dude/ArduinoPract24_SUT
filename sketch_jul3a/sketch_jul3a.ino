#include "DHT.h"

#define MQ135_PIN A0  // Аналоговый пин для датчика MQ-135
#define DHTPIN 2 // номер пина, к которому подсоединен датчик

float a0, a1, a2;

String sensorType;
String unit;

DHT dht(DHTPIN, DHT11);

void setup() {
  Serial.begin(9600);
  pinMode(MQ135_PIN, INPUT);
  pinMode(DHTPIN, OUTPUT);
  dht.begin();

  sensorType = getInput("Введите тип датчика:");
  unit = getInput("Выберите единицы измерения для датчика:");

  EnterPolinom();

  Serial.println("Настройка завершена.");
  Serial.print("Тип датчика: ");
  Serial.println(sensorType);
  Serial.print("Единицы измерения: ");
  Serial.println(unit);
}

void loop() {
  analogExchange();
  delay(2000);  // Увеличен до 2000 мс для удобства чтения данных
}

void digitalExchange() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    return;
  }

  Serial.println("Датчик влажности");
  Serial.print("Влажность: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Температура: ");
  Serial.print(t);
  Serial.println(" *C");
}

void analogExchange() {
  int sensorValue = analogRead(MQ135_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);

  float concentration = polynomialCalibration(voltage, a0, a1, a2);

  Serial.print("Напряжение: ");
  Serial.print(voltage);
  Serial.print(" V\t");
  Serial.print("Концентрация: ");
  Serial.print(concentration);
  Serial.print(" ");
  Serial.println(unit);
}

float polynomialCalibration(float x, float a0, float a1, float a2) {
  return a0 + a1 * x + a2 * x * x;
}

void EnterPolinom() {
  a0 = getFloatInput("Введите коэффициент a0: ");
  a1 = getFloatInput("Введите коэффициент a1: ");
  a2 = getFloatInput("Введите коэффициент a2: ");
}

String getInput(String prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  return Serial.readStringUntil('\n');
}

float getFloatInput(String prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  return Serial.parseFloat();
}
