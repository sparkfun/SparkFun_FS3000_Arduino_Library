/******************************************************************************
  SparkFun_FS3000_Arduino_Library.h
  SparkFun FS3000 Arduino Library - FS3000 Header File
  Pete Lewis @ SparkFun Electronics
  Original Creation Date: August 5th, 2021
  https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library

  Some of this code was copied/tweaked from an Arduino Library for the ADXL345
  Written by E. Roberts @ SparkFun Electronics
  Created: Jul 13, 2016
  https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
  This file defines all registers internal to the ADXL313.

  Development environment specifics:

	IDE: Arduino 1.8.15
	Hardware Platform: SparkFun Redboard Qwiic
	SparkFun Air Velocity Sensor Breakout - FS3000 (Qwicc) Version: 1.0

  Do you like this library? Help support SparkFun. Buy a board!

    SparkFun Air Velocity Sensor Breakout - FS3000 (Qwicc)
    https://www.sparkfun.com/products/18377

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef __SparkFun_FS3000_H__
#define __SparkFun_FS3000_H__

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
#endif

#include "Wire.h"

#define FS3000_TO_READ 5        // Number of Bytes Read:
                                // [0]Checksum, [1]data high, [2]data low, 
                                // [3]generic checksum data, [4]generic checksum data

#define FS3000_DEVICE_ADDRESS 0x28     // Note, the FS3000 does not have an adjustable address.

class FS3000
{
public:
	// FS3000 class constructor
	FS3000();
		
	// begin() -- Initialize the sensor
	// INPUTS:
	// - i2C port (Note, only on "begin()" funtion, for use with I2C com interface)
	//   defaults to Wire, but if hardware supports it, can use other TwoWire ports.
	bool begin(TwoWire &wirePort = Wire); //By default use Wire port
	bool isConnected();
  int readRaw();
  float readMetersPerSecond();
  float readMilesPerHour();
	
private:
	TwoWire *_i2cPort;
  byte _buff[5] ;		//	5 Bytes Buffer
	void readData(byte* _buff);
  bool checksum(byte* _buff, bool debug = false);
  void printHexByte(byte x);
};

#endif // __SparkFun_FS3000_H__ //
