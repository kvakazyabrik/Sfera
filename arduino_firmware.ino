#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "SPI.h"

const int PINS_COUNT = 30;
const int pins [PINS_COUNT] {
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
  53  // 30 - 1014.64
  //38,  31 -- unused --
  //39   32 -- unused --
};
const uint8_t pin_water_sensor = 10;  // номер вывода датчика расхода воды
const int pin_speaker = 11;           // номер вывода динамика
const int BUFFER_SIZE = 16;
char buffer[BUFFER_SIZE];
bool is_water_cooler_warning = true;
bool is_water_notification = false;

inline void check_water_cooler() {
  float varQ = 0.0;                                           // скорость потока воды (л/с)
  uint32_t varL = pulseIn(pin_water_sensor, HIGH, 200000);    // Считываем длительность импульса, но не дольше 0,2 сек.
  if (varL) {                                                 // Если длительность импульса считана, то ...
    float varT = 2.0 * (float)varL / 1000000;                 // Определяем период следования импульсов в сек.
    float varF = 1 / varT;                                    // Определяем частоту следования импульсов в Гц.
    varQ = varF / 450.0f;                                     // Определяем скорость потока воды л/с.
  }
  if (is_water_notification) {
    Serial.println((String)"w" + varQ);
    is_water_notification = false;
  }
  if (!is_water_cooler_warning) {
    return;
  }
  if (varQ == 0) {
    tone(pin_speaker, 3000, 500);
    delay(1000);
  } else {
    noTone(pin_speaker);
  }
}

inline void set_voltage(uint16_t value, uint16_t address_pin) {
  uint16_t data = 0x3000 | value;
  digitalWrite(address_pin, LOW);  // поднятие cs в 1 с нужным аддресом
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.transfer((uint8_t)(data >> 8));
  SPI.transfer((uint8_t)(data & 0xFF));
  SPI.endTransaction();
  digitalWrite(address_pin, HIGH);
}

void setup() {
  SPI.begin();
  for (int i = 0; i < PINS_COUNT ; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }
  Serial.begin(9600);
  while (!Serial) {};
  pinMode(pin_water_sensor, INPUT);
}

void loop() {
  memset(buffer, 0, BUFFER_SIZE);
  int index = 0;
  int index_separator = 0;
  char  data = '*';
read_data:
  check_water_cooler();
  while (Serial.available()) {
    data = Serial.read();
    if (data == '\n') {
      buffer[index++] = '\0';
      break;
    } else {
      if (data == '_') {
        index_separator = index;
        data = '\0';
      }
      buffer[index++] = data;
    }
  }
  if (data != '\n')
    goto read_data;

  if (buffer[0] == 'a' && index_separator > 0) {      ////////// SET VOLTAGE FOR SPECIFIC ADDRESS ////////////
    char* ptr = nullptr;
    unsigned long address = strtoul((const char*)buffer + 1, &ptr, 10);
    unsigned long value = strtoul((const char*)buffer + index_separator + 1, &ptr, 10);
    if (value >= 0 && address > 0 && address <= PINS_COUNT) {
      set_voltage(value, pins[address - 1]);
      Serial.println((String) "a" + address + "_" + value);
    } else {
      Serial.println((String) "e" + 1);
    }
  } else if (buffer[0] == 'f') {                      ////////// TURN OFF ALL DIODS ////////////
    for (int i = 0; i < PINS_COUNT; i++) {
      set_voltage(0, pins[i]);
    }
  } else if (buffer[0] == 'm') {                      ////////// MUTE SOUND WARNING ////////////
    is_water_cooler_warning = false;
    noTone(pin_speaker);
  } else if (buffer[0] == 'u') {                      ////////// UNMUTE SOUND WARNING //////////
    is_water_cooler_warning = true;
  } else if (buffer[0] == 'r') {                      ////////// SOFT RESET CONTROLLER //////////
    asm volatile("jmp 0x00");
  } else if (buffer[0] == 'w') {                      ////////// GET WATER CONSUMPTION //////////
    is_water_notification = true;
  }
}
