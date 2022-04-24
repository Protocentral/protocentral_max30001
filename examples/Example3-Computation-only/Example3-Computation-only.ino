//////////////////////////////////////////////////////////////////////////////////////////
//
//    Demo code for the MAX30001 breakout board
//
//    This example displays heart rate and rr interval through arduino serial port.
//
//    GUI URL: https://github.com/Protocentral/protocentral_openview.git
//
//    Arduino connections:
//
//  |MAX30001 pin label| Pin Function         |Arduino Connection|
//  |----------------- |:--------------------:|-----------------:|
//  | MISO             | Slave Out            |  D12             |
//  | MOSI             | Slave In             |  D11             |
//  | SCLK             | Serial Clock         |  D13             |
//  | CS               | Chip Select          |  D7              |
//  | VCC              | Digital VDD          |  +5V             |
//  | GND              | Digital Gnd          |  Gnd             |
//  | FCLK             | 32K CLOCK            |  -               |
//  | INT1             | Interrupt1           |  02              |
//  | INT2             | Interrupt2           |  -               |
//
//    This software is licensed under the MIT License(http://opensource.org/licenses/MIT).
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   For information on how to use, visit https://github.com/Protocentral/protocentral_max30001
//
/////////////////////////////////////////////////////////////////////////////////////////

#include<SPI.h>
#include "protocentral_max30001.h"

#define INT_PIN 02

MAX30001 max30001;
bool rtorIntrFlag = false;
uint8_t statusReg[3];


void rtorInterruptHndlr(){
  rtorIntrFlag = true;
}


void enableInterruptPin(){

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), rtorInterruptHndlr, CHANGE);
}

void setup()
{
    Serial.begin(115200); //Serial begin

    pinMode(MAX30001_CS_PIN,OUTPUT);
    digitalWrite(MAX30001_CS_PIN,HIGH); //disable device

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);

    bool ret = max30001.max30001ReadInfo();
    if(ret){
      Serial.println("MAX30001 ID Success");
    }else{

      while(!ret){
        //stay here untill the issue is fixed.
        ret = max30001.max30001ReadInfo();
        Serial.println("Failed to read ID, please make sure all the pins are connected");
        delay(5000);
      }
    }

    Serial.println("Initialising the chip ...");
    max30001.max30001BeginRtorMode();   // initialize MAX30001
    enableInterruptPin();
    max30001.max30001RegRead(STATUS, statusReg);
}

void loop()
{
    if(rtorIntrFlag){
      rtorIntrFlag = false;
      max30001.max30001RegRead(STATUS, statusReg);

      if(statusReg[1] & RTOR_INTR_MASK){

        max30001.getHRandRR();   //It will store HR to max30001.heartRate and rr to max30001.RRinterval.
        Serial.print("Heart Rate  = ");
        Serial.println(max30001.heartRate);

        Serial.print("RR interval  = ");
        Serial.println(max30001.RRinterval);
      }
    }
}
