#include <ModbusSlave.h>
#include "ATM90E32.h"
#include <IWatchdog.h>

#define SLAVE_ID 3
#define RS485_BAUDRATE 9600

#define RS485_RX_PIN PA3
#define RS485_TX_PIN PA2
#define RS485_TX_ENABLE_PIN PA1

#define EMON_CS1 PA10
#define EMON_CS2 PA9
#define EMON_CS3 PA4
#define OUTPUT_PIN PB1

const uint16_t lineFreq = 135;//50Hz
const uint16_t pgaGain = 0;// for 5A CT
const uint16_t voltageGain = 11243;
const uint16_t ct1Gain = 13889;
const uint16_t ct2Gain = 13889;
const uint16_t ct3Gain = 13889;
const uint16_t ct4Gain = 13889;
const uint16_t ct5Gain = 13889;
const uint16_t ct6Gain = 13889;
const uint16_t ct7Gain = 13889;
const uint16_t ct8Gain = 13889;
const uint16_t ct9Gain = 13889;

const uint32_t WATCHDOG_TIMEOUT = 10000000; //10s
const uint8_t PERIODICAL_TIMER_FREQUENCY = 1; //1HZ

const uint8_t FREQUENCY = 0;
const uint8_t VOLTAGE = 1;
const uint8_t TEMP1 = 2;
const uint8_t TEMP2 = 3;
const uint8_t TEMP3 = 4;
const uint8_t CURRENT1 = 5;
const uint8_t CURRENT2 = 6;
const uint8_t CURRENT3 = 7;
const uint8_t CURRENT4 = 8;
const uint8_t CURRENT5 = 9;
const uint8_t CURRENT6 = 10;
const uint8_t CURRENT7 = 11;
const uint8_t CURRENT8 = 12;
const uint8_t CURRENT9 = 13;
const uint8_t POWER_FACTOR1 = 14;
const uint8_t POWER_FACTOR2 = 15;
const uint8_t POWER_FACTOR3 = 16;
const uint8_t POWER_FACTOR4 = 17;
const uint8_t POWER_FACTOR5 = 18;
const uint8_t POWER_FACTOR6 = 19;
const uint8_t POWER_FACTOR7 = 20;
const uint8_t POWER_FACTOR8 = 21;
const uint8_t POWER_FACTOR9 = 22;

uint8_t outputState = LOW;
double inputRegister[23] = { 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 };

Modbus slave(SLAVE_ID, RS485_TX_ENABLE_PIN);
ATM90E32 emon1, emon2, emon3;

void updateData() {
  inputRegister[FREQUENCY] = emon1.GetFrequency(); 
  inputRegister[VOLTAGE] = emon1.GetLineVoltageA();
  inputRegister[TEMP1] = emon1.GetTemperature();
  inputRegister[TEMP2] = emon2.GetTemperature();
  inputRegister[TEMP3] = emon3.GetTemperature();
  inputRegister[CURRENT1] = emon1.GetLineCurrentA();
  inputRegister[CURRENT2] = emon1.GetLineCurrentB();
  inputRegister[CURRENT3] = emon1.GetLineCurrentC();
  inputRegister[CURRENT4] = emon2.GetLineCurrentA();
  inputRegister[CURRENT5] = emon2.GetLineCurrentB();
  inputRegister[CURRENT6] = emon2.GetLineCurrentC();
  inputRegister[CURRENT7] = emon3.GetLineCurrentA();
  inputRegister[CURRENT8] = emon3.GetLineCurrentB();
  inputRegister[CURRENT9] = emon3.GetLineCurrentC();
  inputRegister[POWER_FACTOR1] = emon1.GetPowerFactorA();
  inputRegister[POWER_FACTOR2] = emon1.GetPowerFactorB();
  inputRegister[POWER_FACTOR3] = emon1.GetPowerFactorC();
  inputRegister[POWER_FACTOR4] = emon2.GetPowerFactorA();
  inputRegister[POWER_FACTOR5] = emon2.GetPowerFactorB();
  inputRegister[POWER_FACTOR6] = emon2.GetPowerFactorC();
  inputRegister[POWER_FACTOR7] = emon3.GetPowerFactorA();
  inputRegister[POWER_FACTOR8] = emon3.GetPowerFactorB();
  inputRegister[POWER_FACTOR9] = emon3.GetPowerFactorC();
}

void initPeriodicalTimer() {
  HardwareTimer *timer = new HardwareTimer(TIM1);
  timer->setOverflow(PERIODICAL_TIMER_FREQUENCY, HERTZ_FORMAT);
  timer->attachInterrupt(updateData);
  timer->resume();
}

uint8_t readDigitalOut(uint8_t fc, uint16_t address, uint16_t length) {
  slave.writeCoilToBuffer(0, outputState);
  return STATUS_OK;
}

/**
 * Handle Force Single Coil (FC=05) and Force Multiple Coils (FC=15)
 * set digital output pins (coils).
 */
uint8_t writeDigitalOut(uint8_t fc, uint16_t address, uint16_t length) {
  outputState = slave.readCoilFromBuffer(0);
  digitalWrite(OUTPUT_PIN, outputState);
  return STATUS_OK;
}

/**
 * Handle Read Input Registers (FC=04)
 * write back the values from analog in pins (input registers).
 */
uint8_t readEmon(uint8_t fc, uint16_t address, uint16_t length) {
  for (uint16_t i = 0; i <= length; i++) {
    slave.writeRegisterToBuffer(i, (int)(100 * inputRegister[address + i]));
  }
  return STATUS_OK;
}

void setup() {
  pinMode(OUTPUT_PIN, OUTPUT);
  
  slave.cbVector[CB_READ_INPUT_REGISTERS] = readEmon;
  slave.cbVector[CB_READ_COILS] = readDigitalOut;
  slave.cbVector[CB_WRITE_COILS] = writeDigitalOut;

  Serial.setRx(RS485_RX_PIN);
  Serial.setTx(RS485_TX_PIN);
  Serial.begin(RS485_BAUDRATE);
  slave.begin(RS485_BAUDRATE);

  emon1.begin(EMON_CS1, lineFreq, pgaGain, voltageGain, ct1Gain, ct2Gain, ct3Gain);
  emon2.begin(EMON_CS2, lineFreq, pgaGain, voltageGain, ct4Gain, ct5Gain, ct6Gain);
  emon3.begin(EMON_CS3, lineFreq, pgaGain, voltageGain, ct7Gain, ct8Gain, ct9Gain);
    
  initPeriodicalTimer();

  IWatchdog.begin(WATCHDOG_TIMEOUT);
}

void loop() {
  slave.poll();
  IWatchdog.reload();
}
