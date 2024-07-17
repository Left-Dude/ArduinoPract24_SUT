#include "ArduinoJson.h"

#define ANALOGPIN A0  // Аналоговый пин для датчика

float a0, a1, a2; // Коэффициенты квадратичной калибровочной кривой
float lastConcentration = 0.0; // Переменная для хранения предыдущего значения концентрации

StaticJsonDocument<1024> doc; // JSON-документ для хранения данных

// Градуировочная характеристика
const int numPoints = 10; // Количество точек градуировочной кривой

void setup() {
  Serial.begin(9600); // Инициализация последовательного порта для монитора
  pinMode(ANALOGPIN, INPUT); // Установка пина ANALOGPIN на вход

  // Ожидание ввода параметров от внешнего устройства
  waitForParameters();

  // Заполнение и вывод градуировочной кривой
  generateCalibrationCurve();
}

void loop() {
  waitForGradcients(); // Ожидание и обработка данных от внешнего устройства

  delay(500); // Задержка для удобства чтения данных
}

// Функция калибровки: вычисление значения концентрации по калибровочной кривой
float polynomialCalibration(float x, float a0, float a1, float a2) {
  return a0 + a1 * x + a2 * x * x;
}

// Функция обмена данными с аналоговым датчиком
void analogExchange() {
  int sensorValue = analogRead(ANALOGPIN); // Чтение значения с аналогового пина
  doc["sensorValue"] = sensorValue; // Сохранение значения в JSON-документе

  float voltage = sensorValue * (5.0 / 1023.0); // Преобразование значения в напряжение
  doc["voltage"] = voltage; // Сохранение напряжения в JSON-документе

  float concentration = polynomialCalibration(voltage, a0, a1, a2); // Вычисление концентрации
  doc["concentration"] = concentration; // Сохранение концентрации в JSON-документе

  // Вычисление погрешности измерения и сохранение в JSON
  float inaccuracy = abs(concentration - lastConcentration);
  doc["inaccuracy"] = inaccuracy;

  // Обновление предыдущего значения концентрации
  lastConcentration = concentration;
}

// Генерация градуировочной кривой
void generateCalibrationCurve() {
  Serial.println("Градуировочная характеристика:");

  for (int i = 0; i < numPoints; i++) {
    // Заполнение массивов напряжений и концентраций в JSON-документе
    doc["calibrationVoltages"][i] = i * (5.0 / (numPoints - 1));
    doc["calibrationConcentrations"][i] = polynomialCalibration(doc["calibrationVoltages"][i], a0, a1, a2);

    // Вывод данных на монитор последовательного порта
    String unit = doc["unit"];
    
    Serial.print("Напряжение: ");
    Serial.print((float)doc["calibrationVoltages"][i]);
    Serial.print(" V -> Концентрация: ");
    Serial.print((float)doc["calibrationConcentrations"][i]);
    Serial.print(" ");
    Serial.println(unit);
  }
}

// Расчет коэффициентов квадратичной калибровочной кривой по полученному JSON-документу
void calcUnits(JsonDocument dds) {
  int size = dds["calibrationConcentrations"].size(); // Размер массива концентраций
  int random_value_1 = random(0, size); // Генерация случайного индекса 1
  int random_value_2 = random(0, size); // Генерация случайного индекса 2
  int random_value_3 = random(0, size); // Генерация случайного индекса 3

  // Извлечение напряжений и концентраций по случайным индексам
  double x1 = dds["calibrationVoltages"][random_value_1];
  double y1 = dds["calibrationConcentrations"][random_value_1];
  double x2 = dds["calibrationVoltages"][random_value_2];
  double y2 = dds["calibrationConcentrations"][random_value_2];
  double x3 = dds["calibrationVoltages"][random_value_3];
  double y3 = dds["calibrationConcentrations"][random_value_3];

  // Вычисление коэффициентов квадратичной калибровочной кривой методом Гаусса
  double matrix[3][4] = {
    {x1 * x1, x1, 1, y1},
    {x2 * x2, x2, 1, y2},
    {x3 * x3, x3, 1, y3}
  };

  // Прямой ход метода Гаусса для решения системы уравнений
  for (int i = 0; i < 3; i++) {
    // Нормализация текущей строки
    double factor = matrix[i][i];
    for (int j = 0; j < 4; j++) {
      matrix[i][j] /= factor;
    }

    // Обнуление столбца под текущим элементом
    for (int k = i + 1; k < 3; k++) {
      factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }

  // Обратный ход метода Гаусса для решения системы уравнений
  for (int i = 2; i >= 0; i--) {
    for (int k = i - 1; k >= 0; k--) {
      double factor = matrix[k][i];
      for (int j = 0; j < 4; j++) {
        matrix[k][j] -= factor * matrix[i][j];
      }
    }
  }
  // Получение решения системы и установка коэффициентов квадратичной калибровочной кривой
  a2 = matrix[0][3];
  a1 = matrix[1][3];
  a0 = matrix[2][3];

  // Вывод коэффициентов на монитор последовательного порта
  Serial.println("Новые коэффициенты по полученному JSON-документу: ");
  Serial.print("A = ");
  Serial.println(a2);
  Serial.print("B = ");
  Serial.println(a1);
  Serial.print("C = ");
  Serial.println(a0);
}

// Ожидание и получение коэффициентов от внешнего устройства через последовательный порт
void waitForParameters() {
  StaticJsonDocument<256> docJson; // JSON-документ для коэффициентов
  Serial.println("Ожидание коэффициентов от внешнего устройства...");
  while (true) {
    if (Serial.available()) {
      String coefString = Serial.readStringUntil('\n');
      DeserializationError error = deserializeJson(docJson, coefString);
      if (error) {
        Serial.print(F("Ошибка десериализации JSON: "));
        Serial.println(error.f_str());
        continue; // Пропуск текущей итерации и ожидание следующего ввода
      }

      // Проверка наличия всех коэффициентов в JSON-документе
      if (docJson.containsKey("a0") && docJson.containsKey("a1") && docJson.containsKey("a2") && docJson.containsKey("sensorType") && docJson.containsKey("unit")) {
        a0 = docJson["a0"];
        a1 = docJson["a1"];
        a2 = docJson["a2"];

        //Получение и сохранение пользовательского ввода для типа датчика и единиц измерения
        String sensorType = doc["sensorType"] = docJson["sensorType"];
        String unit = doc["unit"] = docJson["unit"];

        Serial.print("Получен тип датчика: ");
        Serial.println(sensorType);
        Serial.print("Получен тип физической величины: ");
        Serial.println(unit);
        Serial.println("Коэффициенты получены:");
        Serial.print("a0 = ");
        Serial.println(a0);
        Serial.print("a1 = ");
        Serial.println(a1);
        Serial.print("a2 = ");
        Serial.println(a2);
        break; // Выход из цикла после получения коэффициентов
      } else {
        Serial.println("JSON-документ не содержит все необходимые коэффициенты. Повторите ввод.");
      }
    }
  }
}

// Ожидание и получение данных градуировочной кривой от внешнего устройства через последовательный порт
void waitForGradcients() {
  StaticJsonDocument<256> inJson; // Входящий JSON-документ
  while (true) {
    if (Serial.available()) {
      String inputString = Serial.readStringUntil('\n'); // Чтение строки из внешнего последовательного порта
      DeserializationError error = deserializeJson(inJson, inputString);
      if (inJson.containsKey("calibrationVoltages") && inJson.containsKey("calibrationConcentrations")) {   
        if (error) {
          // Обработка ошибки десериализации
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          continue;
        }

        // Выполнение калибровки и расчета коэффициентов по полученному JSON-документу
        calcUnits(inJson);
        break;
      } else if (inputString == "getJson") {
        // Если получена команда "getJson", отправляем текущий JSON-документ через внешний порт
        serializeJson(doc, Serial);
        Serial.println((char)0x0A);
      }
    } 
    analogExchange(); // Запуск обмена данными с аналоговым датчиком
  }
}

// Функция получения строки ввода от пользователя через последовательный порт
String getInput(String prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  return Serial.readStringUntil('\n');
}

// Функция получения числового ввода от пользователя через последовательный порт
float getFloatInput(String prompt) {
  Serial.println(prompt);
  while (!Serial.available()) {}
  return Serial.parseFloat();
}
