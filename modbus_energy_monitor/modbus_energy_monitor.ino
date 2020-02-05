#include <ModbusSlave.h>
#include "EmonLib.h"
#include <IWatchdog.h>

const byte SLAVE_ID  = 1;
const byte RS485_BAUDRATE = 9600;

const byte RS485_TX_ENABLE_PIN = PA15;

const byte VOLTAGE_INPUT_PIN = PA0;
const byte CURRENT_INPUT1_PIN = PA1;
const byte CURRENT_INPUT2_PIN = PA2;
const byte CURRENT_INPUT3_PIN = PA3;
const byte CURRENT_INPUT4_PIN = PA4;
const byte CURRENT_INPUT5_PIN = PA5;
const byte CURRENT_INPUT6_PIN = PA6;
const byte CURRENT_INPUT7_PIN = PA7;
const byte CURRENT_INPUT8_PIN = PB0;
const byte CURRENT_INPUT9_PIN = PB1;
const byte OUTPUT_PIN = PB7; //10

Modbus slave(SLAVE_ID, RS485_TX_ENABLE_PIN);
EnergyMonitor ct1, ct2, ct3, ct4, ct5, ct6, ct7, ct8, ct9;

//https://learn.openenergymonitor.org/electricity-monitoring/ctac/calibration?redirected=true
const float I_CAL = 45.4; // (1000 turns / 22 Ohm burden) = 45.4
const float V_CAL = 510.0;

const float PHASE_SHIFT = 1.7;
const int NO_OF_SAMPLES = 1662;
const byte NO_OF_HALF_WAVELENGTH = 30;
const int EMON_TIMEOUT = 2000;

const int WATCHDOG_TIMEOUT = 10000000; //10s

int vrms;

const byte CURRENT_CHANNELS = 9;
int power[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void setup() {
  IWatchdog.begin(WATCHDOG_TIMEOUT);
  
  pinMode(VOLTAGE_INPUT_PIN, INPUT);
  pinMode(CURRENT_INPUT1_PIN, INPUT);
  pinMode(CURRENT_INPUT2_PIN, INPUT);
  pinMode(CURRENT_INPUT3_PIN, INPUT);
  pinMode(CURRENT_INPUT4_PIN, INPUT);
  pinMode(CURRENT_INPUT5_PIN, INPUT);
  pinMode(CURRENT_INPUT6_PIN, INPUT);
  pinMode(CURRENT_INPUT7_PIN, INPUT);
  pinMode(CURRENT_INPUT8_PIN, INPUT);
  pinMode(CURRENT_INPUT9_PIN, INPUT);
  pinMode(OUTPUT_PIN, OUTPUT);
  
  slave.cbVector[CB_READ_INPUT_REGISTERS] = readAnalogIn;
  slave.cbVector[CB_WRITE_COILS] = writeDigitalOut;

  Serial.setRx(PA10);
  Serial.setTx(PA9);
  Serial.begin(RS485_BAUDRATE);
  slave.begin(RS485_BAUDRATE);

  ct1.current(CURRENT_INPUT1_PIN, I_CAL);
  ct2.current(CURRENT_INPUT2_PIN, I_CAL);
  ct3.current(CURRENT_INPUT3_PIN, I_CAL);
  ct4.current(CURRENT_INPUT4_PIN, I_CAL);
  ct5.current(CURRENT_INPUT5_PIN, I_CAL);
  ct6.current(CURRENT_INPUT6_PIN, I_CAL);
  ct7.current(CURRENT_INPUT7_PIN, I_CAL);
  ct8.current(CURRENT_INPUT8_PIN, I_CAL);
  ct9.current(CURRENT_INPUT9_PIN, I_CAL);

  ct1.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct2.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct3.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct4.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct5.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct6.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct7.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct8.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
  ct9.voltage(VOLTAGE_INPUT_PIN, V_CAL, PHASE_SHIFT);
 
  IWatchdog.reload(); 
}

void loop() {
  ct1.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  vrms = (int)ct1.Vrms*100;
  power[0] = (int)ct1.realPower;
  
  ct2.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[1] = (int)ct2.realPower;
 
  ct3.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[2] = (int)ct3.realPower;
 
  ct4.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[3] = (int)ct4.realPower;
 
  ct5.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[4] = (int)ct5.realPower;
 
  ct6.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[5] = (int)ct6.realPower;
 
  ct7.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[6] = (int)ct7.realPower;
 
  ct8.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[7] = (int)ct8.realPower;
 
  ct9.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[8] = (int)ct9.realPower;
  
  slave.poll();
  
  IWatchdog.reload();
}

/**
 * Handle Force Single Coil (FC=05) and Force Multiple Coils (FC=15)
 * set digital output pins (coils).
 */
 
uint8_t writeDigitalOut(uint8_t fc, uint16_t address, uint16_t length) {
  for (int i = 0; i < length; i++) {
     digitalWrite(address + i, slave.readCoilFromBuffer(i));
  }
  return STATUS_OK;
}

/**
 * Handle Read Input Registers (FC=04)
 * write back the values from analog in pins (input registers).
 */

uint8_t readAnalogIn(uint8_t fc, uint16_t address, uint16_t length) {
    for (int i = 0; i < CURRENT_CHANNELS; i++) {
        slave.writeRegisterToBuffer(i, power[address + i]);
    }
    return STATUS_OK;
}
