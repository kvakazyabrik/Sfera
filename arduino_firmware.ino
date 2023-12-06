#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SPI.h"
#include "unordered_map"
/*#define MCP4921_CS_PIN    29*/

uint8_t pinSensor = 10;  // Определяем номер вывода Arduino, к которому подключён датчик расхода воды.
int freq = 11;           // Объявляем динамик
float varQ;              // Объявляем переменную для хранения рассчитанной скорости потока воды (л/с).
float varV;              // Объявляем переменную для хранения рассчитанного объема воды (л).

//  инициализация пинов на плате для светодиодов
unordered_map <int,unsigned long> pins;
pins ={
        {1,44},
        {2,45},
        {3,42},
        {4,40},
        {5,41},
        {6,43},
        {7,13},
        {8,30},
        {9,36},
        {10,23},
        {11,25},
        {12,27},
        {13,26},
        {14,24},
        {15,22},
        {16,33},
        {17,32},
        {18,35},
        {19,34},
        {20,37},
        {21,29},
        {22,28},
        {23,31},
        {24,47},
        {25,46},
        {26,49},
        {27,48},
        {28,8},
        {29,9},
        {30,53},
        {31,38},
        {32,39}
    };

char SerialData = 0;  //  шапка сом - порта
char SerialBuffer[16];
int BufferIndex = 0;
volatile bool endOfTransmission = false;
char* ptr;
unsigned long address_buf = 0;
int address = 0;
int flags = 0;
unsigned long val = 0;

// forward declaration
void setup();
void clear_after_end_of_transmission();
setVoltage(uint16_t value, uint16_t address_pin);

inline void clear_after_end_of_transmission() {
  memset(SerialBuffer, 0, sizeof(SerialBuffer));                    // обнуление буфера
  BufferIndex = 0;
  endOfTransmission = 0;
  address_buf = 0;
};

void setup() {
  SPI.begin();
  for (int i = 22; i <= 50; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(50, OUTPUT);
  pinMode(53, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(13, OUTPUT);


  Serial.begin(9600);  // Инициируем передачу данных в монитор последовательного порта.
  while (!Serial) {};
  pinMode(pinSensor, INPUT);  // Конфигурируем вывод к которому подключён датчик, как вход.
  varQ = 0;
  varV = 0;  // Обнуляем все переменные.

}  // 51 и 52 не учавствуют

void setVoltage(uint16_t value, uint16_t address_pin) {
  // value= 20;
  uint16_t data = 0x3000 | value;
  digitalWrite(address_pin, LOW);  // поднятие cs в 1 с нужным аддресом
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.transfer((uint8_t)(data >> 8));
  Serial.println((uint8_t)(data >> 8), HEX);
  SPI.transfer((uint8_t)(data & 0xFF));
  Serial.println((uint8_t)(data & 0xFF), HEX);
  SPI.endTransaction();
  digitalWrite(address_pin, HIGH);
}

void loop() {
start_of_loop:
  if (Serial.available() > 0) {  //   общение по com - порту
    SerialData = Serial.read();
    if (SerialData == '\n') {
      SerialBuffer[BufferIndex++] = '\0';
      endOfTransmission = true;

    } else {
      SerialBuffer[BufferIndex++] = SerialData;
      endOfTransmission = false;
    }
  }

  if (endOfTransmission) {
    switch (SerialBuffer[0]) {  // по написанному в буффере выбор адреса светодиода

      case 0x61:  // a
        if (isDigit(SerialBuffer[1]))
          address_buf = strtoul((const char*)SerialBuffer + 1, &ptr, 10);   //Преобразование строки в значение типа unsigned long int
        else {
          Serial.write("invalid pin command");                        // ответ от контроллера
          clear_after_end_of_transmission();
          goto start_of_loop; 
        } 
        address = pins.at(address_buf);
        break;
      
      case 0x76:  // v
        if (isDigit(SerialBuffer[1]))
          val = strtoul((const char*)SerialBuffer + 1, &ptr, 10);   //Преобразование строки в значение типа unsigned long int
        else {
          Serial.write("invalid value command");                        // ответ от контроллера
          clear_after_end_of_transmission();
          goto start_of_loop;
        }
        break;
      
      case 0x66:  // f
        flags = 0;
        break;
    }
    clear_after_end_of_transmission();
  }

  if (flags == 0) {
    for (int i = 22; i <= 49; i++) {
      address = i;
      val = 0;
      setVoltage(val, address);
      address = 0;// зачем это делать?
    }
    address = 8;
    val = 0; 
    setVoltage(val, address);
    address = 9;
    setVoltage(val, address);
    address = 13;
    setVoltage(val, address);
    address = 53;
    setVoltage(val, address);
    flags = 1;
    val = 0;
    address = 0;
  }

  if (val != 0 & address != 0) {
    setVoltage(val, address);
    val = 0;
    address = 0;
    Serial.write("complete");                        // ответ от контроллера
    Serial.println("    ");
    Serial.println("    ");
  }

  varQ = 0;                                          // Сбрасываем скорость потока воды.
  uint32_t varL = pulseIn(pinSensor, HIGH, 200000);  // Считываем длительность импульса, но не дольше 0,2 сек.
  if (varL) {                                        // Если длительность импульса считана, то ...
    float varT = 2.0 * (float)varL / 1000000;        // Определяем период следования импульсов в сек.
    float varF = 1 / varT;                           // Определяем частоту следования импульсов в Гц.
    varQ = varF / 450.0f;                            // Определяем скорость потока воды л/с.
    varV += varQ * varT;                             // Определяем объем воды л.
  }                                                  //
  //  Выводим рассчитанные данные:                                //
  //Serial.println((String) "Объем " + varV + "л, скорость " + (varQ * 60.0f) + "л/м.");

  if (varQ == 0) {     // Если расход = 0, те не раб, то:
    tone(freq, 1);  //включаем на 1000

    asm volatile("jmp 0x00"); // reset до состояния нормальной работы
  } else {
    noTone(freq);
  }
}
