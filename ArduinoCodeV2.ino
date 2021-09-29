// Controls MOT deflector power supply. There are 4 channels: X, Y, Z axial deflectors and an Extraction voltage. The box has two TTL inputs that apply or disable the deflector voltages and extraction voltage condition. 
// If the deflector TTL signal is high and the extraction TTL signal is low, voltages are applied to the X, Y and Z deflector pairs as set by front panel pots or 3 12bit DAC channels. 
// If the extractin TTL signal is high and the deflector TTL signal is low, all of the deflector plates are set to -0.7V except for the detector front plate which is biased to -8V to attract ions. This is at 4095 applied to the Extraction DAC channel. It should scale with this.
// The deflector voltages are set by front panel pots or by DAC depending on front panel switch positions. The DACs are controlled by sending the arduino serial messages from PC.
// The protocol is:
// ID - ask for name of arduino. In this case it is "DP"
// GP - get mains power status. 1 for on and 0 for off.
// SX, SY, SZ - Set deflector pair to value between 0 and 4095. Zero volts is around 2040 or so
// SE - Set extraction voltage. 4095 ~-8V on front plate and +0.7V on all other plates.
// 
// X is DACXE channel 0
// Y is DACZY channel 1 Updated 5/12/19: Y is vertical.
// Z is DACZY channel 0 Updated 5/12/19: Z is horizontal.
// E is DACXE channel 1

#include "MilliTimer.h"
#include "SerialChecker.h"
#include "MCP4822_SPI.h"

const char ID[] = "DP"; // Stands for Deflector Power supplies

// Serial settings
SerialChecker sc; // default baudrate is 250000
// DAC settings
MCP4822 dacXE(3); // chip select pin 3
MCP4822 dacZY(4); // chip select pin 4

void setup(){
    sc.init();
    sc.enableACKNAK();
    dacXE.init();
    dacZY.init();
    pinMode(2, INPUT); // Monitors the mains power status of the power supply box. Opto isolated and inverted signal.
    // setZeroVoltsOut(); // Zeros the DACs. Leave commented for now in case serial connection is accidentally reset. This way, last settings are preserved.
}

void loop(){
    if(sc.check()){
        if(sc.contains("ID")){ 
            Serial.println(ID);
        }
        else if(sc.contains("$ID")){
            Serial.println(ID);
        }
        else if(sc.contains("GP")){ // Get the mains power status
            Serial.println(!digitalRead(2));
        }
        else if(sc.getMsgLen() > 2){
            if(sc.contains("SX")){ 
                dacXE.setVoltage(0, sc.toInt16());
                sc.sendACK();
            }
            else if(sc.contains("SY")){ 
                dacZY.setVoltage(1, sc.toInt16()); 
                sc.sendACK();
            }
            else if(sc.contains("SZ")){ 
                dacZY.setVoltage(0, sc.toInt16()); 
                sc.sendACK();
            }
            else if(sc.contains("SE")){ 
                dacXE.setVoltage(1, sc.toInt16());
                sc.sendACK();
            }
            else{
                sc.sendNAK();
            }
        }
        else{
            sc.sendNAK();
        }
    }
}

void setZeroVoltsOut(){
    dacXE.setVoltage(0, 2072); // X Updated 12/12/19
    dacZY.setVoltage(1, 2034); // Y Updated 12/12/19
    dacZY.setVoltage(0, 2048); // Z Updated 12/12/19
    dacXE.setVoltage(1, 2057); // E Updated 12/12/19
}