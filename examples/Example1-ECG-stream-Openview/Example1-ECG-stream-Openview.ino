//////////////////////////////////////////////////////////////////////////////////////////
//
//    Demo code for the MAX30001 breakout board
//
//    This example plots the ECG through serial UART on openview processing GUI.
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

#include <SPI.h>
#include "protocentral_max30001.h"

#define MAX30001_CS_PIN         7
#define MAX30001_DELAY_SAMPLES  8 // Time between consecutive samples

#define PC_OV_CMDIF_PKT_START_1 0x0A
#define PC_OV_CMDIF_PKT_START_2 0xFA
#define PC_OV_CMDIF_TYPE_DATA 0x02
#define PC_OV_CMDIF_PKT_STOP 0x0B

#define DATA_LEN 0x0C
#define ZERO 0

volatile char DataPacket[DATA_LEN];
const char DataPacketFooter[2] = {ZERO, PC_OV_CMDIF_PKT_STOP};
const char DataPacketHeader[5] = {PC_OV_CMDIF_PKT_START_1, PC_OV_CMDIF_PKT_START_2, DATA_LEN, ZERO, PC_OV_CMDIF_TYPE_DATA};

uint8_t data_len = 0x0C;

MAX30001 max30001(MAX30001_CS_PIN);

signed long ecg_data;
signed long bioz_data;

void sendData(signed long ecg_sample, signed long bioz_sample)
{

  DataPacket[0] = ecg_sample;
  DataPacket[1] = ecg_sample >> 8;
  DataPacket[2] = ecg_sample >> 16;
  DataPacket[3] = ecg_sample >> 24;

  DataPacket[4] = bioz_sample;
  DataPacket[5] = bioz_sample >> 8;
  DataPacket[6] = bioz_sample >> 16;
  DataPacket[7] = bioz_sample >> 24;

  DataPacket[8] = 0x00; // max30001.heartRate;
  DataPacket[9] = 0x00; // max30001.heartRate >> 8;
  DataPacket[10] = 0x00;
  DataPacket[11] = 0x00;
  
  // Send packet header (in ProtoCentral OpenView format)
  for (int i = 0; i < 5; i++)
  {
    Serial.write(DataPacketHeader[i]);
  }

  // Send the data payload
  for (int i = 0; i < DATA_LEN; i++) // transmit the data
  {
    Serial.write(DataPacket[i]);
  }

  // Send packet footer (in ProtoCentral OpenView format)
  for (int i = 0; i < 2; i++)
  {
    Serial.write(DataPacketFooter[i]);
  }
}

void setup()
{
  Serial.begin(57600);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  bool ret = max30001.max30001ReadInfo();
  if (ret)
  {
    Serial.println("MAX 30001 read ID Success");
  }
  else
  {
    while (!ret)
    {
      // Keep trying every 5 seconds
      ret = max30001.max30001ReadInfo();
      Serial.println("Failed to read ID, please make sure all the pins are connected");
      delay(5000);
    }
  }

  max30001.BeginECGBioZ(); // Start sampling in ECG and BioZ modes                     
}

void loop()
{
  ecg_data = max30001.getECGSamples();
  // max30001.getHRandRR();
  bioz_data = max30001.getBioZSamples();
  sendData(ecg_data, bioz_data);

  delay(8*2);
}