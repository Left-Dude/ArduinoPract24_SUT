#include "DHT.h"
#include "ArduinoJson.h"

#define MQ135_PIN A0  // Аналоговый пин для датчика MQ-135
#define DHTPIN 2 // номер пина, к которому подсоединен датчик

float a0, a1, a2;

String sensorType;
String unit;

JsonDocument doc;

DHT dht(DHTPIN, DHT11);

// Градуировочная характеристика
const int numPoints = 10;
float calibrationVoltages[numPoints];
float calibrationConcentrations[numPoints];

void setup() {
  Serial.begin(9600);
  pinMode(MQ135_PIN, INPUT);
  pinMode(DHTPIN, OUTPUT);
  dht.begin();

  doc["sensor"] = sensorType = getInput("Введите тип датчика:");
  doc["unit"] = unit = getInput("Выберите единицы измерения для датчика:");

  EnterPolinom();

  // Заполнение градуировочной характеристики
  generateCalibrationCurve();

  Serial.println("Настройка завершена.");
  Serial.print("Тип датчика: ");
  Serial.println(sensorType);
  Serial.print("Единицы измерения: ");
  Serial.println(unit);

  // Вывод коэффициентов

  calcUnits();
}

void loop() {

  analogExchange();
  serializeJson(doc, Serial);
  delay(20000);  // Увеличен до 2000 мс для удобства чтения данных
}

void analogExchange() {
  int sensorValue = analogRead(MQ135_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  doc["voltage"] = voltage;
  float concentration = polynomialCalibration(voltage, a0, a1, a2);
  doc["concentration"] = concentration;

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

void generateCalibrationCurve() {
  Serial.println("Градуировочная характеристика:");

  for (int i = 0; i < numPoints; i++) {
    calibrationVoltages[i] = i * (5.0 / (numPoints - 1)); // Разделение диапазона 0-5V на равные части
    calibrationConcentrations[i] = polynomialCalibration(calibrationVoltages[i], a0, a1, a2);
    Serial.print("Напряжение: ");
    Serial.print(calibrationVoltages[i]);
    Serial.print(" V -> Концентрация: ");
    Serial.print(calibrationConcentrations[i]);
    Serial.print(" ");
    Serial.println(unit);
  }
}

void calcUnits() {
  float x1 = calibrationVoltages[1];
  float x2 = calibrationVoltages[4];
  float x3 = calibrationVoltages[9];
  float y1 = calibrationConcentrations[1];
  float y2 = calibrationConcentrations[4];
  float y3 = calibrationConcentrations[9];

  float a[3][4] = {
    {1, x1, x1*x1, y1},
    {1, x2, x2*x2, y2},
    {1, x3, x3*x3, y3}
  };

  for (int i = 0; i < 3; i++) {
    // Normalize the current row
    float f = a[i][i];
    for (int j = 0; j < 4; j++) {
      a[i][j] /= f;
    }

    // Eliminate the current column
    for (int k = 0; k < 3; k++) {
      if (k == i) continue;
      f = a[k][i];
      for (int j = 0; j < 4; j++) {
        a[k][j] -= f * a[i][j];
      }
    }
  }

  float b = a[0][3];
  float K = a[1][3];
  float L = a[2][3];

  Serial.println("Полученные коэффициенты:");
  Serial.print("b = ");
  Serial.println(b);
  Serial.print("K = ");
  Serial.println(K);
  Serial.print("L = ");
  Serial.println(L);
}