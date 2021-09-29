#ifndef _MCP4822_SPI_H
#define _MCP4822_SPI_H

#include <Arduino.h>
#include <SPI.h>

// Bit banged class to send updates to MCP4822. Only for High gain (x2) at the moment.
// Pins:
// 1 Vdd    5V
// 2 !CS
// 3 SCK
// 4 SDI
// 5 VoutA
// 6 Vss    0V
// 7 VoutB
// 8 !LDAC  Tie low to update immediately.

class MCP4822{
    private:
        // Private functions and variables here.  They can only be accessed
        // by functions within the class.
        uint8_t _cs;
        uint8_t _MSByte;
        uint8_t _LSByte;
        uint8_t _fineRes = 0;
        bool _gain;

    public:
        // Public functions and variables.  These can be accessed from
        // outside the class.
        MCP4822(uint8_t cs);
        MCP4822(uint8_t cs, bool gain);
    
        void init();
        uint8_t init(uint8_t fineRes);
        void setVoltage(bool channel, uint16_t voltage);
        void setVoltage(uint32_t voltage);
        uint8_t setFineResolution(uint8_t fineRes); // use both channels of the DAC in a voltage divider with buffer follower to gain higher resolution. For example, to gain an extra 10 bits, make a voltage divider where the fine channel has 2^10=1024 times the resistance compared to the coarse channel. Now use the fine DAC's output in multiples of 2^12/2^10=2^2=4 steps. Each time the fine output overflows 2^12-1, increment the coarse step. 
        void setGain(bool gain);
        void setChipSelect(uint8_t csPin);
        uint8_t getBitResolution();
        uint32_t getNumberOfSteps();
};
#endif


// To do:
// 1. Document
// 2. change to more appropriate SPItransfer thing that can take an array or a uint16_t