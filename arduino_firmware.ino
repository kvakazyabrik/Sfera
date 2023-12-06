#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SPI.h"
/*#define MCP4921_CS_PIN    29*/

uint8_t pinSensor = 10;  // Определяем номер вывода Arduino, к которому подключён датчик расхода воды.
int freq = 11;           // Объявляем динамик
float varQ;              // Объявляем переменную для хранения рассчитанной скорости потока воды (л/с).
float varV;              // Объявляем переменную для хранения рассчитанного объема воды (л).
const int S1 = 22;       //  инициализация пинов на плате для светодиодов
const int S2 = 23;
const int S3 = 24;
const int S4 = 25;
const int S5 = 26;
const int S6 = 27;
const int S7 = 28;
const int S8 = 29;
const int S9 = 30;
const int S10 = 31;
const int S11 = 32;
const int S12 = 33;
const int S13 = 34;
const int S14 = 35;
const int S15 = 36;
const int S16 = 37;
const int S17 = 38;
const int S18 = 39;
const int S19 = 40;
const int S20 = 41;
const int S21 = 42;
const int S22 = 43;
const int S23 = 44;
const int S24 = 45;
const int S25 = 46;
const int S26 = 47;
const int S27 = 48;
const int S28 = 49;
const int S29 = 9;
const int S30 = 8;
const int S31 = 13;
const int S32 = 53;

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
          Serial.write("invalid command");                        // ответ от контроллера
          clear_after_end_of_transmission();
        }
        switch (address_buf) {
          case 0x01:
            address = S23;
            break;

          case 0x02:
            address = S24;
            break;

          case 0x03:
            address = S21;
            break;

          case 0x04:
            address = S19;
            break;

          case 0x05:
            address = S20;
            break;

          case 0x06:
            address = S22;
            break;

          case 0x07:
            address = S31;
            break;

          case 0x08:
            address = S9;
            break;

          case 0x09:
            address = S15;
            break;

          case 0x0A:
            address = S2;
            break;

          case 0x0B:
            address = S4;
            break;

          case 0x0C:
            address = S6;
            break;

          case 0x0D:
            address = S5;
            break;

          case 0x0E:
            address = S3;
            break;

          case 0x0F:
            address = S1;
            break;

          case 0x10:
            address = S12;
            break;

          case 0x11:
            address = S11;
            break;

          case 0x12:
            address = S14;
            break;

          case 0x13:
            address = S13;
            break;

          case 0x14:
            address = S16;
            break;

          case 0x15:
            address = S8;
            break;

          case 0x16:
            address = S7;
            break;

          case 0x17:
            address = S10;
            break;

          case 0x18:
            address = S26;
            break;

          case 0x19:
            address = S25;
            break;

          case 0x1A:
            address = S28;
            break;

          case 0x1B:
            address = S27;
            break;

          case 0x1C:
            address = S30;
            break;

          case 0x1D:
            address = S29;
            break;

          case 0x1E:
            address = S32;
            break;

          case 0x1F:
            address = S17;
            break;

          case 0x20:
            address = S18;
            break;
        }

        break;
      case 0x76:  // v
        if (isDigit(SerialBuffer[1]))
          val = strtoul((const char*)SerialBuffer + 1, &ptr, 10);   //Преобразование строки в значение типа unsigned long int
        else {
          Serial.write("invalid command");                        // ответ от контроллера
          clear_after_end_of_transmission();
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
      address = 0;
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
