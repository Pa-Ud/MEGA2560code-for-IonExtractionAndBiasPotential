#include "MCP4822_SPI.h"

MCP4822::MCP4822(uint8_t cs){
    _cs = cs;
    _gain = true;
}

MCP4822::MCP4822(uint8_t cs, bool gain){
    _cs = cs;
    _gain = gain;
}

void MCP4822::init(){
    SPI.begin();
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
}

uint8_t MCP4822::init(uint8_t fineRes){
    _fineRes = fineRes;
    init();
    return _fineRes + 12;
}

void MCP4822::setVoltage(bool channel, uint16_t voltage){
    // <A or B><-><Gain 1 or 2><shutdown or active><D11><D10><D9><D8><D7><D6><D5><D4><D3><D2><D1><D0>
    _MSByte = 0b00010000 | (channel << 7); // set command bits
    _MSByte |= !_gain << 5; // 0 is high gain and 1 is low gain.
    _MSByte |= 0b00001111 & (voltage >> 8); // get the 4 most significant bits of voltage and add them to most significant byte message
    _LSByte = 0b11111111 & voltage; // get 8 least significant bits
    digitalWrite(_cs, LOW);
    (void) SPI.transfer(_MSByte); // Send command bits and 4 MSB of voltage
    (void) SPI.transfer(_LSByte); // Send 8 LSB of voltage
    digitalWrite(_cs, HIGH);
}

void MCP4822::setVoltage(uint32_t voltage){
    /*
    fineBits = 10
    coarseBits = 12
    totalBits = fineBits + coarseBits
    numberOfSteps = 1 << totalBits
    print("numberOfSteps: ", numberOfSteps)
    print("coarseSteps: ", 1 << coarseBits)
    print("fineSteps: ", 1 << fineBits)
    level = 4194304
    coarse = level >> fineBits
    fine = level - (coarse << fineBits)
    coarseValue = coarse
    fineValue = fine * (1 << (coarseBits - fineBits))
    print("coarse: ", coarse)
    print("fine: ", fine)
    print("coarseValue: ", coarse)
    print("fineValue:", fineValue)
    print("total: ", fine + coarseValue * (1<<fineBits))
    */
    uint16_t coarse = voltage >> _fineRes;
    uint16_t fine = voltage - (coarse << _fineRes);
    fine *= 1 << (12 - _fineRes);
    // Serial.print(coarse);
    // Serial.print(", ");
    // Serial.println(fine);
    setVoltage(0, fine);
    setVoltage(1, coarse);
}

uint8_t MCP4822::setFineResolution(uint8_t fineRes){
    _fineRes = fineRes;
    return _fineRes + 12; // (1 << _fineRes + 12) gives the number of steps
}

void MCP4822::setGain(bool gain){
    _gain = gain;
}
void MCP4822::setChipSelect(uint8_t csPin){
    _cs = csPin;
}

uint8_t MCP4822::getBitResolution(){
    return _fineRes + 12;
}

uint32_t MCP4822::getNumberOfSteps(){
    return uint32_t(1) << _fineRes + 12;
}