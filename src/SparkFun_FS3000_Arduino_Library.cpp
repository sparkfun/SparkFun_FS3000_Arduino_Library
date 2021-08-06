/******************************************************************************
  SparkFun_FS3000_Arduino_Library.cpp
  SparkFun FS3000 Arduino Library - main source file
  Pete Lewis @ SparkFun Electronics
  Original Creation Date: August 5th, 2021
  https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library

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

#include "Arduino.h"
#include "SparkFun_FS3000_Arduino_Library.h"
#include <Wire.h>

FS3000::FS3000() 
{

}

//Initializes the sensor (no settings to adjust)
//Returns false if sensor is not detected
boolean FS3000::begin(TwoWire &wirePort)
{
  	_i2cPort = &wirePort;
  	if (isConnected() == false) // Check for sensor by verifying ACK response
    	return (false); 
  	return (true); //We're all setup!
}

//Returns true if I2C device ack's
boolean FS3000::isConnected()
{
  	_i2cPort->beginTransmission((uint8_t)FS3000_DEVICE_ADDRESS);
  	if (_i2cPort->endTransmission() != 0)
    	return (false); //Sensor did not ACK
  	return (true);
}

/*************************** READ RAW **************************/
/*  Read from sensor, checksum, return raw data (409-3686)     */
uint16_t FS3000::readRaw() {
    readData(_buff);
    bool checksum_result = checksum(_buff, false); // debug off
    if(checksum_result == false) // checksum error
    {
      return 9999; 
    }
    
    uint16_t airflowRaw = 0;
    uint8_t data_high_byte = _buff[1];
    uint8_t data_low_byte = _buff[2];

    // The flow data is a 12-bit integer. 
    // Only the least significant four bits in the high byte are valid.
    // clear out (mask out) the unnecessary bits
    data_high_byte &= (0b00001111);
    
    // combine high and low bytes, to express sensors 12-bit value
    airflowRaw |= data_low_byte;
    airflowRaw |= (data_high_byte << 8);

    return airflowRaw;
}

/*************************** READ METERS PER SECOND****************/
/*  Read from sensor, checksum, return m/s (0-7.23)     */
float FS3000::readMetersPerSecond() {
    float airflowMps = 0.0;
    int airflowRaw = readRaw();

    // convert from Raw readings to m/s.
    // There is an output curve on datasheet page 8.
    // it has 9 data points, and it's not a perfectly straight line.
    // let's create two arrays to contain the conversion data.
    // Then we can find where our reading lives in the array, 
    // then we can consider it a straight line conversion graph between each data point.

    float mpsDataPoint[9] = {0, 1.07, 2.01, 3.00, 3.97, 4.96, 5.98, 6.99, 7.23};
    int rawDataPoint[9] =  {409, 915, 1522, 2066, 2523, 2908, 3256, 3572, 3686};

    // find out where our raw data fits in the arrays
    int data_position = 0;

    for( int i = 0 ; i < 9 ; i++)
    {
        if (airflowRaw > rawDataPoint[i])
        {
          data_position = i;
          //Serial.println(data_position);
        }
    }

    // set limits on min and max.
    // if we are at or below 409, we'll bypass conversion and report 0.
    // if we are at or above 3686, we'll bypass conversion and report 7.23
    if(airflowRaw <= 409) return 0;
    if(airflowRaw >= 3686) return 7.23;

    // look at where we are between the two data points in the array.
    // now use the percentage of that window to calculate m/s

    // calculate the percentage of the window we are at.
    // using the data_position, we can find the "edges" of the data window we are in
    // and find the window size
    
    int window_size = (rawDataPoint[data_position+1] - rawDataPoint[data_position]);
    //Serial.print("window_size: ");
    //Serial.print(window_size);

    // diff is the amount (difference) above the bottom of the window
    int diff = (airflowRaw - rawDataPoint[data_position]);
    //Serial.print("diff: ");
    //Serial.print(diff);
    float percentage_of_window = ((float)diff / (float)window_size);
    //Serial.print("\tpercent: ");
    //Serial.print(percentage_of_window);

    // calculate window size from MPS data points
    float window_size_mps = (mpsDataPoint[data_position+1] - mpsDataPoint[data_position]);
    //Serial.print("\twindow_size_mps: ");
    //Serial.print(window_size_mps);

    // add percentage of window_mps to mps.
    airflowMps = mpsDataPoint[data_position] + (window_size_mps*percentage_of_window);

    return airflowMps;
}

/*************************** READ MILES PER HOUR****************/
/*  Read from sensor, checksum, return mpg (0-33ish)     */
float FS3000::readMilesPerHour() {
    return (readMetersPerSecond()*2.2369362912);
}

/*************************** READ DATA **************************/
/*                Read 5 bytes from sensor, put it at a pointer (given as argument)                  */
void FS3000::readData(uint8_t* _buff) {
    // Wire.reqeustFrom contains the beginTransmission and endTransmission in it. 
	Wire.requestFrom(FS3000_DEVICE_ADDRESS, 4);  // Request 4 Bytes

  uint8_t i = 0;
	while(Wire.available())
	{
		_buff[i] = Wire.read();				// Receive Byte
    i += 1;
	}
  //Serial.print("i:");
  //Serial.println(i);
}

/****************************** CHECKSUM *****************************
 * Check to see that the CheckSum is correct, and data is good
 * Return true if all is good, return false if something is off   
 * The entire response from the FS3000 is 5 bytes.
 * [0]Checksum
 * [1]data high
 * [2]data low
 * [3]generic checksum data
 * [4]generic checksum data
 */

bool FS3000::checksum(uint8_t* _buff, bool debug) {
    uint8_t sum = 0;
    for (int i = 1; i <= 4; i++) {
        sum += uint8_t(_buff[i]);
    }

    if(debug) Serial.print("Sum of received data bytes                       = ");
    if(debug) printHexByte(sum);

    uint8_t calculated_cksum = -sum;
    if(debug) Serial.print("Calculated checksum                              = ");
    if(debug) printHexByte(calculated_cksum);
    
    uint8_t crcbyte = _buff[0];
    if(debug) Serial.print("Received checksum byte                           = ");
    if(debug) printHexByte(crcbyte);
    
    uint8_t overall = sum+crcbyte;
    if(debug) Serial.print("Sum of received data bytes and received checksum = ");
    if(debug) printHexByte(overall);
    if(debug) Serial.println();

    if (overall != 0x00)
    {
        return false;
    }
    return true;
}

void FS3000::printHexByte(uint8_t x)
{
    Serial.print("0x");
    if (x < 16) {
        Serial.print('0');
    }
    Serial.print(x, HEX);
    Serial.println();
}