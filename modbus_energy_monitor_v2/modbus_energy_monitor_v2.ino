#include <ModbusSlave.h>
#include <ATM90E36.h>
#include <IWatchdog.h>

const byte SLAVE_ID = 3;
const int RS485_BAUDRATE = 9600;

const byte RS485_TX_ENABLE_PIN = 5;//PA15
const byte EMON_CS1 = 6;
const byte EMON_CS2 = 7;
const byte EMON_CS3 = 10;
const PinName OUTPUT_PIN = PB_7;

const int WATCHDOG_TIMEOUT = 10000000; //10s
const uint8_t PERIODICAL_TIMER_FREQUENCY = 1; //1HZ

const byte FREQUENCY = 0;
const byte VOLTAGE = 1;
const byte CURRENT1 = 2;
const byte CURRENT2 = 3;
const byte CURRENT3 = 4;
const byte CURRENT4 = 5;
const byte CURRENT5 = 6;
const byte CURRENT6 = 7;
const byte CURRENT7 = 8;
const byte CURRENT8 = 9;
const byte CURRENT9 = 10;
const byte POWER1 = 11;
const byte POWER2 = 12;
const byte POWER3 = 13;
const byte POWER4 = 14;
const byte POWER5 = 15;
const byte POWER6 = 16;
const byte POWER7 = 17;
const byte POWER8 = 18;
const byte POWER9 = 19;
const byte POWER_FACTOR1 = 20;
const byte POWER_FACTOR2 = 21;
const byte POWER_FACTOR3 = 22;
const byte POWER_FACTOR4 = 23;
const byte POWER_FACTOR5 = 24;
const byte POWER_FACTOR6 = 25;
const byte POWER_FACTOR7 = 26;
const byte POWER_FACTOR8 = 27;
const byte POWER_FACTOR9 = 28;

double inputRegister[29] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

Modbus slave(SLAVE_ID, RS485_TX_ENABLE_PIN);
ATM90E36 emon1(EMON_CS1), emon2(EMON_CS2), emon3(EMON_CS3);

void setup() {
  IWatchdog.begin(WATCHDOG_TIMEOUT);
  
  pinMode(OUTPUT_PIN, OUTPUT);
  
  slave.cbVector[CB_READ_INPUT_REGISTERS] = readEmon;
  slave.cbVector[CB_WRITE_COILS] = writeDigitalOut;

  Serial.setRx(PA10);
  Serial.setTx(PA9);
  Serial.begin(RS485_BAUDRATE);
  slave.begin(RS485_BAUDRATE);

  emon1.begin();
  emon2.begin();
  emon3.begin();
  
  initPeriodicalTimer();
}

void loop() {
  slave.poll(); 
  IWatchdog.reload();
}

void initPeriodicalTimer() {
  HardwareTimer *timer = new HardwareTimer(TIM2);
  timer->setOverflow(PERIODICAL_TIMER_FREQUENCY, HERTZ_FORMAT);
  timer->attachInterrupt(updateData);
  timer->resume();
}

void updateData() {
  inputRegister[FREQUENCY] = emon1.GetFrequency(); 
  inputRegister[VOLTAGE] = emon1.GetLineVoltageA();
  inputRegister[CURRENT1] = emon1.GetLineCurrentA();
  inputRegister[CURRENT2] = emon1.GetLineCurrentB();
  inputRegister[CURRENT3] = emon1.GetLineCurrentC();
  inputRegister[CURRENT4] = emon2.GetLineCurrentA();
  inputRegister[CURRENT5] = emon2.GetLineCurrentB();
  inputRegister[CURRENT6] = emon2.GetLineCurrentC();
  inputRegister[CURRENT7] = emon3.GetLineCurrentA();
  inputRegister[CURRENT8] = emon3.GetLineCurrentB();
  inputRegister[CURRENT9] = emon3.GetLineCurrentC();
  inputRegister[POWER1] = emon1.GetActivePowerA();
  inputRegister[POWER2] = emon1.GetActivePowerB();
  inputRegister[POWER3] = emon1.GetActivePowerC();
  inputRegister[POWER4] = emon2.GetActivePowerA();
  inputRegister[POWER5] = emon2.GetActivePowerB();
  inputRegister[POWER6] = emon2.GetActivePowerC();
  inputRegister[POWER7] = emon3.GetActivePowerA();
  inputRegister[POWER8] = emon3.GetActivePowerB();
  inputRegister[POWER9] = emon3.GetActivePowerC();
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

/**
 * Handle Force Single Coil (FC=05) and Force Multiple Coils (FC=15)
 * set digital output pins (coils).
 */
uint8_t writeDigitalOut(uint8_t fc, uint16_t address, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    digitalWrite(address + i, slave.readCoilFromBuffer(i));
  }
  return STATUS_OK;
}

/**
 * Handle Read Input Registers (FC=04)
 * write back the values from analog in pins (input registers).
 */
uint8_t readEmon(uint8_t fc, uint16_t address, uint16_t length) {
  for (uint16_t i = 0; i <= length; i++) {
    slave.writeRegisterToBuffer(i, (int)(inputRegister[address + i] * 10));
  }
  return STATUS_OK;
}
