#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SPI.h"

inline int get_address(const unsigned long& address) {
  if (address == 0x1)  // 1  - 397.755
    return 44;
  if (address == 0x2)  // 2  - 404.563
    return 45;
  if (address == 0x3)  // 3  - 412.732
    return 42;
  if (address == 0x4)  // 4  - 427.213
    return 40;
  if (address == 0x5)  // 5  - 433.773
    return 41;
  if (address == 0x6)  // 6  - 445.531
    return 43;
  if (address == 0x7)  // 7  - 452.71
    return 13;
  if (address == 0x8)  // 8  - 461.993
    return 30;
  if (address == 0x9)  // 9  - 473.628
    return 36;
  if (address == 0x0A) // 10 - 505.066
    return 23;
  if (address == 0x0B) // 11 - 534.276
    return 25;
  if (address == 0x0C) // 12 - 549.252
    return 27;
  if (address == 0x0D) // 13 - 560.516
    return 26;
  if (address == 0x0E) // 14 - 588.983
    return 24;
  if (address == 0x0F) // 15 - 595.543
    return 22;
  if (address == 0x10) // 16 - 607.549
    return 33;
  if (address == 0x11) // 17 - 627.105
    return 32;
  if (address == 0x12) // 18 - 640.101
    return 35;
  if (address == 0x13) // 19 - 658.791
    return 34;
  if (address == 0x14) // 20 - 679.089
    return 37;
  if (address == 0x15) // 21 - 725.133
    return 29;
  if (address == 0x16) // 22 - 745.802
    return 28;
  if (address == 0x17) // 23 - 775.013
    return 31;
  if (address == 0x18) // 24 - 800.634
    return 47;
  if (address == 0x19) // 25 - 820.066
    return 46;
  if (address == 0x1A) // 26 - 839.127
    return 49;
  if (address == 0x1B) // 27 - 893.463
    return 48;
  if (address == 0x1C) // 28 - 931.461
    return 8;
  if (address == 0x1D) // 29 - 967.107
    return 9;
  if (address == 0x1E) // 30 - 1014.64
    return 53;
  if (address == 0x1F) // 31 - -- unused --
    return 38;
  if (address == 0x20) // 32 - -- unused --
    return 39;
};

uint8_t pinSensor = 10;  // номер вывода датчика расхода воды
int freq = 11;           // номер вывода динамика
float varQ = 0.0;        // скорость потока воды (л/с)
float varV = 0.0;        // объем воды (л)

char SerialData = 0;
char SerialBuffer[16];
char* ptr = nullptr;
int BufferIndex = 0;
int address = 0;
unsigned long val = 0;
unsigned long address_buf = 0;
volatile bool endOfTransmission = false;

inline void clear_buffer();
inline void clear_address_and_value();
inline void clear_buffer_and_variables();
inline void check_water_cooler();
inline void set_voltage(uint16_t value, uint16_t address_pin);

inline void clear_buffer() {
  memset(SerialBuffer, 0, sizeof(SerialBuffer));
  BufferIndex = 0;
  endOfTransmission = false;
  address_buf = 0;
}

inline void clear_address_and_value() {
  val = 0;
  address = 0;
}

inline void clear_buffer_and_variables() {
  clear_buffer();
  clear_address_and_value();
}

inline void check_water_cooler() {
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

inline void set_voltage(uint16_t value, uint16_t address_pin) {
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

  Serial.begin(9600);
  while (!Serial) {};
  pinMode(pinSensor, INPUT);
}

void loop() {
start_of_loop:
  check_water_cooler();
  if (Serial.available() > 0) {
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
    switch (SerialBuffer[0]) {
////////////////////////////////////////////////////////////////// SET ADDRESS //////////////////////////////////////////////////////////////////
      case 0x61:  // a --> выбор адреса
        if (isDigit(SerialBuffer[1]))
          address_buf = strtoul((const char*)SerialBuffer + 1, &ptr, 10);
        else {
          Serial.write("invalid pin command");
          clear_buffer_and_variables();
          goto start_of_loop;
        }
        address = get_address(address_buf);
        clear_buffer();
        goto start_of_loop;
//////////////////////////////////////////////////////////////////// SET VALUE /////////////////////////////////////////////////////////////////
      case 0x76:  // v --> установка значения
        if (isDigit(SerialBuffer[1]))
          val = strtoul((const char*)SerialBuffer + 1, &ptr, 10);
        else {
          Serial.write("invalid value command");
        }
        if (val != 0 & address != 0) {
          set_voltage(val, address);
          Serial.write("set value is completed.......\n\n");
        }
        clear_buffer_and_variables();
        goto start_of_loop;
///////////////////////////////////////////////////////////////////// TURN OFF ALL DIODS //////////////////////////////////////////////////////////////////
      case 0x66:  // f --> выключение всех светодиодов
        for (int i = 22; i <= 49; i++) {
          set_voltage(0, i);
        }
        set_voltage(0, 8);
        set_voltage(0, 9);
        set_voltage(0, 13);
        set_voltage(0, 53);
        clear_buffer_and_variables();
        goto start_of_loop;
    }
  }
}
