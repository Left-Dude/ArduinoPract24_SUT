#include "ArduinoJson.h"

#define ANALOGPIN A0  // Аналоговый пин для датчика MQ-135
#define DHTPIN 2 // номер пина, к которому подсоединен датчик

float a0, a1, a2;
float A, B, C;

String sensorType;
String unit;

JsonDocument doc;

// Градуировочная характеристика
const int numPoints = 10;
float calibrationVoltages[numPoints];
float calibrationConcentrations[numPoints];

void setup() {
  Serial.begin(9600);
  pinMode(ANALOGPIN, INPUT);

  doc["sensor"] = sensorType = getInput("Введите тип датчика:");
  doc["unit"] = unit = getInput("Выберите тип физической величины:");

  EnterPolinom();

  // DEBUGING
  //
  // JsonDocument ddc;
  // for (int i = 0; i < 13; i++) {
  //   ddc["calibrationVoltages"][i] = random(0,10);
  //   ddc["calibrationConcentrations"][i] = random(0,10);
  // }

  // Заполнение градуировочной характеристики
  generateCalibrationCurve();


  Serial.println("Настройка завершена.");
  Serial.print("Тип датчика: ");
  Serial.println(sensorType);
  Serial.print("Единицы измерения: ");
  Serial.println(unit);

  // Коллибровка. Получение коэффициентов
  calcUnits(calibrationVoltages, calibrationConcentrations);
  //calcUnits(ddc);
}

void loop() {
  analogExchange();

  serializeJson(doc, Serial);
  Serial.println((char)0x0A);
  delay(10000);  // Увеличен до 2000 мс для удобства чтения данных
}

float polynomialCalibration(float x, float a0, float a1, float a2) {
  return a0 + a1 * x + a2 * x * x;
}

void analogExchange() {
  int sensorValue = analogRead(ANALOGPIN);
  doc["sensorValue"] = sensorValue;

  float voltage = sensorValue * (5.0 / 1023.0);
  doc["voltage"] = voltage;

  float concentration1 = polynomialCalibration(voltage, a0, a1, a2);
  doc["concentration1"] = concentration1;

  float concentration2 = polynomialCalibration(voltage, a0, a1, a2);

  Serial.print("Напряжение: ");
  Serial.print(voltage);
  Serial.print(" V\t");
  Serial.print("Концентрация: ");
  Serial.print(concentration1);
  Serial.print(" ");
  Serial.println(unit);

  Serial.print("Новая Концентрация: ");
  Serial.print(concentration2);
  Serial.print(" ");
  Serial.println(unit);
}

void generateCalibrationCurve(){
  Serial.println("Градуировочная характеристика:");

  for (int i = 0; i < numPoints; i++) {
    doc["calibrationVoltages"][i] = calibrationVoltages[i] = i * (5.0 / (numPoints - 1)); // Разделение диапазона 0-5V на равные части
    doc["calibrationConcentrations"][i] = calibrationConcentrations[i] = polynomialCalibration(doc["calibrationVoltages"][i], a0, a1, a2);
    Serial.print("Напряжение: ");
    Serial.print((float)doc["calibrationVoltages"][i]);
    Serial.print(" V -> Концентрация: ");
    Serial.print((float)doc["calibrationConcentrations"][i]);
    Serial.print(" ");
    Serial.println(unit);
  }
}

void calcUnits(JsonDocument dds) {
  // float Voltages [numPoints], Concentrations [numPoints];
  // for (int i = 0; i < numPoints; i++) {
  //   Voltages[i] = dds["calibrationVoltages"][i];
  //   Concentrations[i] = dds["calibrationConcentrations"][i];
  // }
  int sz = dds["calibrationConcentrations"].size()-1;
  int random_value_1 = random(0,sz);
  int random_value_2 = random(0,sz);
  int random_value_3 = random(0,sz); 

  float x1 = dds["calibrationVoltages"][random_value_1];
  float y1 = dds["calibrationConcentrations"][random_value_1];
  float x2 = dds["calibrationVoltages"][random_value_2];
  float y2 = dds["calibrationConcentrations"][random_value_2];
  float x3 = dds["calibrationVoltages"][random_value_3];
  float y3 = dds["calibrationConcentrations"][random_value_3];

  // Вычисление коэффициентов A, B и C для квадратичной функции y = Ax^2 + Bx + C
  float matrix[3][4] = {
    {x1 * x1, x1, 1, y1},
    {x2 * x2, x2, 1, y2},
    {x3 * x3, x3, 1, y3}
  };

  // Прямой ход метода Гаусса
  for (int i = 0; i < 3; i++) {
    // Нормализация строки
    float factor = matrix[i][i];
    for (int j = 0; j < 4; j++) {
      matrix[i][j] /= factor;
    }

    // Обнуление столбца
    for (int k = i + 1; k < 3; k++) {
      factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }

  // Обратный ход метода Гаусса
  for (int i = 2; i >= 0; i--) {
    for (int k = i - 1; k >= 0; k--) {
      float factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }

  // Решение системы
  float A = matrix[0][3];
  float B = matrix[1][3];
  float C = matrix[2][3];

  Serial.println("Полученные коэффициенты:");
  Serial.print("A = ");
  Serial.println(A);
  Serial.print("B = ");
  Serial.println(B);
  Serial.print("C = ");
  Serial.println(C);
}

void calcUnits(float *_calibrationVoltages, float *_calibrationConcentrations) {
  int random_value_1 = random(0,10);
  int random_value_2 = random(0,10);
  int random_value_3 = random(0,10); 

  float x1 = _calibrationVoltages[random_value_1];
  float y1 = _calibrationConcentrations[random_value_1];
  float x2 = _calibrationVoltages[random_value_2];
  float y2 = _calibrationConcentrations[random_value_2];
  float x3 = _calibrationVoltages[random_value_3];
  float y3 = _calibrationConcentrations[random_value_3];

  // Вычисление коэффициентов A, B и C для квадратичной функции y = Ax^2 + Bx + C
  float matrix[3][4] = {
    {x1 * x1, x1, 1, y1},
    {x2 * x2, x2, 1, y2},
    {x3 * x3, x3, 1, y3}
  };

  // Прямой ход метода Гаусса
  for (int i = 0; i < 3; i++) {
    // Нормализация строки
    float factor = matrix[i][i];
    for (int j = 0; j < 4; j++) {
      matrix[i][j] /= factor;
    }

    // Обнуление столбца
    for (int k = i + 1; k < 3; k++) {
      factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }

  // Обратный ход метода Гаусса
  for (int i = 2; i >= 0; i--) {
    for (int k = i - 1; k >= 0; k--) {
      float factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }

  // Решение системы
  float A = matrix[0][3];
  float B = matrix[1][3];
  float C = matrix[2][3];

  Serial.println("Полученные коэффициенты:");
  Serial.print("A = ");
  Serial.println(A);
  Serial.print("B = ");
  Serial.println(B);
  Serial.print("C = ");
  Serial.println(C);
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