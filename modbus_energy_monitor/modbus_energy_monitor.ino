#include <ModbusSlave.h>
//#include "EmonLib.h"

#define SLAVE_ID 1
#define RS485_BAUDRATE 9600

#define RS485_TX_ENABLE_PIN PA15
#define OUTPUT1_PIN PB7

Modbus slave(SLAVE_ID, RS485_TX_ENABLE_PIN);
//EnergyMonitor emon;

void setup() {
  pinMode(OUTPUT1_PIN, OUTPUT);
 
  slave.cbVector[CB_WRITE_COILS] = writeDigitalOut;

  Serial.setRx(PA10);
  Serial.setTx(PA9);
  Serial.begin(RS485_BAUDRATE);
  slave.begin(RS485_BAUDRATE);

 // emon.voltage(2, 510, 1.7);  // Voltage: input pin, calibration, phase_shift
  //emon.current(1, 111.1);  
}

void loop() {
 // emon.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
 // emon.serialprint();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
 // float realPower       = emon.realPower;        //extract Real Power into variable
 // float apparentPower   = emon.apparentPower;    //extract Apparent Power into variable
 // float powerFActor     = emon.powerFactor;      //extract Power Factor into Variable
//  float supplyVoltage   = emon.Vrms;             //extract Vrms into Variable
 // float Irms            = emon.Irms;             //extract Irms into Variable
  slave.poll();
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
