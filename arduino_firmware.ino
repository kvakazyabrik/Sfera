#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SPI.h"

const int PINS_COUNT = 32;
int pins [PINS_COUNT] {
  44, // 1  - 397.755
  45, // 2  - 404.563
  42, // 3  - 412.732
  40, // 4  - 427.213
  41, // 5  - 433.773
  43, // 6  - 445.531
  13, // 7  - 452.71
  30, // 8  - 461.993
  36, // 9  - 473.628
  23, // 10 - 505.066
  25, // 11 - 534.276
  27, // 12 - 549.252
  26, // 13 - 560.516
  24, // 14 - 588.983
  22, // 15 - 595.543
  33, // 16 - 607.549
  32, // 17 - 627.105
  35, // 18 - 640.101
  34, // 19 - 658.791
  37, // 20 - 679.089
  29, // 21 - 725.133
  28, // 22 - 745.802
  31, // 23 - 775.013
  47, // 24 - 800.634
  46, // 25 - 820.066
  49, // 26 - 839.127
  48, // 27 - 893.463
  8,  // 28 - 931.461
  9,  // 29 - 967.107
  53, // 30 - 1014.64
  38, // 31 - -- unused --
  39  // 32 - -- unused --
};
uint8_t pinSensor = 10;  // номер вывода датчика расхода воды

char SerialBuffer[16];
char* ptr = nullptr;
int address = 0;
unsigned long address_buf = 0;

inline void clear_buffer();
inline void clear_address_and_value();
inline void clear_buffer_and_variables();
inline void check_water_cooler();
inline void set_voltage(uint16_t value, uint16_t address_pin);

inline void clear_buffer() {
  memset(SerialBuffer, 0, sizeof(SerialBuffer));
  address_buf = 0;
}

inline void clear_address_and_value() {
  address = 0;
}

inline void clear_buffer_and_variables() {
  clear_buffer();
  clear_address_and_value();
}

inline void check_water_cooler() {
  int freq = 11;                                       // номер вывода динамика
  float varQ = 0.0;                                    // скорость потока воды (л/с)
  float varV = 0.0;                                    // объем воды (л)
 
  uint32_t varL = pulseIn(pinSensor, HIGH, 200000);    // Считываем длительность импульса, но не дольше 0,2 сек.
  if (varL) {                                          // Если длительность импульса считана, то ...
    float varT = 2.0 * (float)varL / 1000000;          // Определяем период следования импульсов в сек.
    float varF = 1 / varT;                             // Определяем частоту следования импульсов в Гц.
    varQ = varF / 450.0f;                              // Определяем скорость потока воды л/с.
    varV += varQ * varT;                               // Определяем объем воды л.
  }
  //Serial.println((String) "Объем " + varV + "л, скорость " + (varQ * 60.0f) + "л/м.");
  if (varQ == 0) {
    tone(freq, 1);
    asm volatile("jmp 0x00"); // soft reset
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
  for (int i = 0; i < PINS_COUNT ; i++) {
    pinMode(pins[i], OUTPUT);
  }
  Serial.begin(9600);
  while (!Serial) {};
  pinMode(pinSensor, INPUT);
  clear_buffer_and_variables();
}

void loop() {
start_of_loop:
  check_water_cooler();
  int index = 0;
  char data;
  while (Serial.available()) {
    data = Serial.read();
    Serial.println(data);
    if (data == '\n') {
      SerialBuffer[index++] = '\0';
      break;
    } else {
      SerialBuffer[index++] = data;
    }
  }
    /////////////////////////////////////////////////////////////////// SET ADDRESS ///////////////////
      if(SerialBuffer[0]=='a'){ // a --> выбор адреса
        if (isDigit(SerialBuffer[1]))
          address_buf = strtoul((const char*)SerialBuffer + 1, &ptr, 10);
        else {
          Serial.write("invalid pin command");
          clear_buffer_and_variables();
          goto start_of_loop;
        }
        address = pins[address_buf];
        clear_buffer();
        goto start_of_loop;
        }else if(SerialBuffer[0]=='v'){
/////////////////////////////////////////////////////////////////// SET VALUE /////////////////////
        // v --> установка значения
        unsigned long val = 0;
        if (isDigit(SerialBuffer[1]))
          val = strtoul((const char*)SerialBuffer + 1, &ptr, 10);
        else {
          Serial.write("invalid value command");
        }
        if (val != 0 & address != 0) {
          set_voltage(val, address);
          Serial.println((String) "set value is completed......."+val);
        }
        clear_buffer_and_variables();
        goto start_of_loop;
        }else if(SerialBuffer[0]=='f'){
/////////////////////////////////////////////////////////////////// TURN OFF ALL DIODS ////////////
        // f --> выключение всех светодиодов
        Serial.println("f ----------- case");
        for (int i = 0; i < PINS_COUNT; i++) {
          set_voltage(0, pins[i]);
        }
        clear_buffer_and_variables();
        goto start_of_loop;
        } 
  }
