#include <ModbusSlave.h>
#include <EmonLib.h>
#include <IWatchdog.h>

const byte SLAVE_ID = 3;
const int RS485_BAUDRATE = 9600;

const byte RS485_TX_ENABLE_PIN = 5;//PA15

const PinName VOLTAGE_INPUT_PIN = PA_0;
const PinName CURRENT_INPUT1_PIN = PA_1;
const PinName CURRENT_INPUT2_PIN = PA_2;
const PinName CURRENT_INPUT3_PIN = PA_3;
const PinName CURRENT_INPUT4_PIN = PA_4;
const PinName CURRENT_INPUT5_PIN = PA_5;
const PinName CURRENT_INPUT6_PIN = PB_1;
const PinName CURRENT_INPUT7_PIN = PB_0;
const PinName CURRENT_INPUT8_PIN = PA_7;
const PinName CURRENT_INPUT9_PIN = PA_6;
const PinName OUTPUT_PIN = PB_7;

Modbus slave(SLAVE_ID, RS485_TX_ENABLE_PIN);
EnergyMonitor ct1, ct2, ct3, ct4, ct5, ct6, ct7, ct8, ct9;

const float I_CAL = 45.45;
const float V_CAL = 1808.04;

const float PHASE_SHIFT = 1.7;
const byte NO_OF_HALF_WAVELENGTH = 30;
const int EMON_TIMEOUT = 2000;

const int WATCHDOG_TIMEOUT = 10000000; //10s

float vrms;

const byte CURRENT_CHANNELS = 2;
float power[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

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

void getCTData(byte channel, EnergyMonitor &ct) {
  ct.calcVI(NO_OF_HALF_WAVELENGTH, EMON_TIMEOUT);
  power[channel] = (float)ct.realPower;
}

void getChannelData(byte channel) {
  switch (channel) {
    case 0: getCTData(0, ct1); break;
    case 1: getCTData(1, ct2); break;
    case 2: getCTData(2, ct3); break;
    case 3: getCTData(3, ct4); break;
    case 4: getCTData(4, ct5); break;
    case 5: getCTData(5, ct6); break;
    case 6: getCTData(6, ct7); break;
    case 7: getCTData(7, ct8); break;
    case 8: getCTData(8, ct9); break;
  }
}

void loop() {
  for (int channel = 0; channel < CURRENT_CHANNELS; channel++) {
    getChannelData(channel);
  }
  vrms = ct1.Vrms;
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
  slave.writeRegisterToBuffer(0, (int)(vrms * 10));
  int i;
  for (i = 1; i <= CURRENT_CHANNELS; i++) {
    slave.writeRegisterToBuffer(i, (int)(power[address + i - 1] * 10));
  }
  return STATUS_OK;
}
