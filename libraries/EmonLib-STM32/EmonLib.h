/*
  Emon.h - Library for openenergymonitor
  Created by Trystan Lea, April 27 2010
  GNU GPL
  modified to use up to 12 bits ADC resolution (ex. Arduino Due)
  by boredman@boredomprojects.net 26.12.2013
  Low Pass filter for offset removal replaces HP filter 1/1/2015 - RW
*/

#ifndef EmonLib_h
#define EmonLib_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

// define theoretical vref calibration constant for use in readvcc()
// 1100mV*1024 ADC steps http://openenergymonitor.org/emon/node/1186
// override in your code with value for your specific AVR chip
// determined by procedure described under "Calibrating the internal reference voltage" at
// http://openenergymonitor.org/emon/buildingblocks/calibration
#ifndef READVCC_CALIBRATION_CONST
#define READVCC_CALIBRATION_CONST 1126400L
#endif

// to enable 12-bit ADC resolution on Arduino Due,
// include the following line in main sketch inside setup() function:
//  analogReadResolution(ADC_BITS);
// otherwise will default to 10 bits, as in regular Arduino-based boards.
#if defined(__arm__)
#define ADC_BITS    12
#else
#define ADC_BITS    10
#endif

#define ADC_COUNTS  (1<<ADC_BITS)


class EnergyMonitor
{
  public:

    void voltage(PinName _inPinV, double _VCAL, double _PHASECAL);
    void current(PinName _inPinI, double _ICAL);

    void calcVI(unsigned int crossings, unsigned int timeout);
    
    long readVcc();
    //Useful value variables
    double realPower,
      powerFactor,
      Vrms;

  private:

    //Set Voltage and current input pins
    PinName inPinV;
    PinName inPinI;
    //Calibration coefficients
    //These need to be set in order to obtain accurate results
    double VCAL;
    double ICAL;
    double PHASECAL;

    //--------------------------------------------------------------------------------------
    // Variable declaration for emon_calc procedure
    //--------------------------------------------------------------------------------------

    double offsetV;                          //Low-pass filter output
    double offsetI;                          //Low-pass filter output
};

#endif
