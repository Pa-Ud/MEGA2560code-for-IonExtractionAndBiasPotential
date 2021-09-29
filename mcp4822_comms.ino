#include <SPI.h>
#include "MCPDAC.h"

//DAC channel mapping:
//DAC0A = X deflectors        Zero volts out at 2071
//DAC0B = Extraction voltage  Zero volts out at 2055
//DAC1A = Z deflectors        Zero volts out at 2054
//DAC1B = Y deflectors        Zero volts out at 2040


int incomingByte = 0;   // for incoming serial data
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
int interval = 2; // how many milliseconds to wait before checking serial port and updating DACs

bool currentPower;
bool previousPower;
MCPDACClass MCPDACs[3];
byte message[3];
unsigned int dacSelect;
unsigned int dacVoltage;
unsigned int dacVoltages[4]={2071,2055,2054,2040};//initialise so opamps will output zero.
bool dacUpdateRequested;
int dacMessageLength;

void setup() 
{
  Serial.begin(115200);
  //setup DACs
  // CS on pin 3 for first DAC
  initDACs();

  pinMode(2, INPUT); // pin 2 is used to read power state of main board. Opto isolated so must invert.
  currentPower = digitalRead(2);
  currentPower = !currentPower;
  previousPower = currentPower; 
  Serial.println("Connected");
  Serial.print("Mains Power Status: ");
  if (currentPower){
    Serial.println("On");
  }
  else{
    Serial.println("Off");
  }
}

void loop() 
{
  currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    dacUpdate();
    checkPowerStatus();
  }
}
 
void dacUpdate()
{
  dacUpdateRequested=0;
  dacMessageLength=0;
  while(Serial.available() && dacMessageLength<3) {
    // read the incoming byte:
    message[dacMessageLength] = Serial.read();
    dacMessageLength++;
    if(dacMessageLength>=3){dacUpdateRequested = 1;}
  }
  if (dacUpdateRequested) {
    dacSelect=(unsigned int)message[0]; //first byte selects the output supply to change, 0 to 5
    dacVoltage=(unsigned int)message[1];//two bytes are sent that need to be combined into an int. Read first byte and convert to int.
    dacVoltage = dacVoltage << 8;       //Next bit shift by 1 byte to the right since the highbyte was sent first and is currently a factor of 2^8=256 too small
    dacVoltage |= (unsigned int)message[2]; //Finally, OR the second (lowerbyte) byte with the shifted combined output int in order to combine them in to the original int value
    Serial.print("Received: ");
    Serial.print(dacVoltage);
    Serial.print(", on channel: ");
    Serial.println(dacSelect);
    switch(dacSelect){
      case 0:
        MCPDACs[0].setVoltage(CHANNEL_A,dacVoltage&0x0fff);
        break;
      case 1:
        MCPDACs[0].setVoltage(CHANNEL_B,dacVoltage&0x0fff);
        break;
      case 2:
        MCPDACs[1].setVoltage(CHANNEL_A,dacVoltage&0x0fff);
        break;
      case 3:
        MCPDACs[1].setVoltage(CHANNEL_B,dacVoltage&0x0fff);
        break;
    }
    dacVoltages[dacSelect]=dacVoltage; //update dacVoltages array
  }
}

void checkPowerStatus()
{
  //If the unit is switched on, the DACs will default to 0V output. This will cause the opamp outputs to hit their rails.
  //This function checks to see if the 5V rail has gone from 0V (off) to 5V (on) and updates all the DACs with the values
  //currently stored in the dacVoltage array.
  currentPower = digitalRead(2);
  currentPower = !currentPower; //invert because the power is checked via an opto-isolater that inverts signal
  if(currentPower == false and previousPower == true){
    previousPower = false;
    Serial.println("Mainboard power lost.");
  }
  else if(currentPower == true and previousPower == false){
    previousPower = currentPower;
    initDACs();
    Serial.println("Mainboard power restored. DACs reset.");
  }
}

void initDACs()
{
  for (int i=0;i<2;i++){
    MCPDACs[i].begin(i+3); //chip select pins are 3,4
    MCPDACs[i].setGain(CHANNEL_A,GAIN_HIGH);  // Set the gain to "HIGH" mode - 0 to 4096mV.
    MCPDACs[i].setGain(CHANNEL_B,GAIN_HIGH);
    MCPDACs[i].shutdown(CHANNEL_A,false);
    MCPDACs[i].shutdown(CHANNEL_B,false);
    // set chips to 2.048V so that opamps are zeroed
    MCPDACs[i].setVoltage(CHANNEL_A,dacVoltages[2*i]);
    MCPDACs[i].setVoltage(CHANNEL_B,dacVoltages[2*i+1]);
  }
}
