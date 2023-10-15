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

  Artemis Redboard @ 400KHz (Core v2.1.0) 
  (note, v2.1.1 has a known issue with clock stretching at 100KHz)

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
bool FS3000::begin(TwoWire &wirePort)
{
  	_i2cPort = &wirePort;
  	if (isConnected() == false) // Check for sensor by verifying ACK response
    	return (false); 
  	return (true); //We're all setup!
}

//Returns true if I2C device ack's
bool FS3000::isConnected()
{
  	_i2cPort->beginTransmission((uint8_t)FS3000_DEVICE_ADDRESS);
  	if (_i2cPort->endTransmission() != 0)
    	return (false); //Sensor did not ACK
  	return (true);
}

/*************************** SET RANGE OF SENSOR ****************/
/*  There are two varieties of this sensor (1) FS3000-1005 (0-7.23 m/sec)
and (2) FS3000-1015 (0-15 m/sec)     

Valid input arguments are:
AIRFLOW_RANGE_7_MPS
AIRPLOW_RANGE_15_MPS

Note, this also sets the datapoints (from the graphs in the datasheet pages 6 and 7).
These datapoints are used to convert from raw values to m/sec - and then mph.

*/
void FS3000::setRange(uint8_t range) {
    _range = range;
    const float mpsDataPoint_7_mps[9] = {0, 1.07, 2.01, 3.00, 3.97, 4.96, 5.98, 6.99, 7.23}; // FS3000-1005 datapoints
    const int rawDataPoint_7_mps[9] =  {409, 915, 1522, 2066, 2523, 2908, 3256, 3572, 3686}; // FS3000-1005 datapoints

    const float mpsDataPoint_15_mps[13] = {0, 2.00, 3.00, 4.00, 5.00, 6.00, 7.00, 8.00, 9.00, 10.00, 11.00, 13.00, 15.00}; // FS3000-1015 datapoints
    const int rawDataPoint_15_mps[13] =  {409, 1203, 1597, 1908, 2187, 2400, 2629, 2801, 3006, 3178, 3309, 3563, 3686}; // FS3000-1015 datapoints

    if (_range == AIRFLOW_RANGE_7_MPS)
    {
      for (int i = 0 ; i < 9 ; i++)
      {
        _mpsDataPoint[i] = mpsDataPoint_7_mps[i];
        _rawDataPoint[i] = rawDataPoint_7_mps[i];
      }
    }
    else if (_range == AIRFLOW_RANGE_15_MPS)
    {
      for (int i = 0 ; i < 13 ; i++)
      {
        _mpsDataPoint[i] = mpsDataPoint_15_mps[i];
        _rawDataPoint[i] = rawDataPoint_15_mps[i];
      }
    }
}

/*************************** READ RAW **************************/
/*  Read from sensor, checksum, return raw data (409-3686)     */
uint16_t FS3000::readRaw() {
    readData(_buff);
    bool checksum_result = checksum(_buff, false); // debug off
    if(checksum_result == false) // checksum error
    {
      //return 9999; 
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

    // Find out how many datapoints we have from the conversion graphs (depends on the range).
    // note the datasheet pages 6 and 7 show graphs with difference amounts of datapoints
    // for each range.
    uint8_t dataPointsNum = 9; // Default to FS3000_1005 AIRFLOW_RANGE_7_MPS
    if (_range == AIRFLOW_RANGE_7_MPS)
    {
      dataPointsNum = 9;
    }
    else if (_range == AIRFLOW_RANGE_15_MPS)
    {
      dataPointsNum = 13;
    }

    // find out where our raw data fits in the arrays
    int data_position = 0;

    for( int i = 0 ; i < dataPointsNum ; i++) // cound be an array of datapoints 9 or 13 long...
    {
        if (airflowRaw > _rawDataPoint[i])
        {
          data_position = i;
          //Serial.println(data_position);
        }
    }

    // set limits on min and max.
    // if we are at or below 409, we'll bypass conversion and report 0.
    // if we are at or above 3686, we'll bypass conversion and report max (7.23 or 15)
    if(airflowRaw <= 409) return 0;
    if(airflowRaw >= 3686) 
    {
      if(_range == AIRFLOW_RANGE_7_MPS) return 7.23;
      if(_range == AIRFLOW_RANGE_15_MPS) return 15.00;
    }

    // look at where we are between the two data points in the array.
    // now use the percentage of that window to calculate m/s

    // calculate the percentage of the window we are at.
    // using the data_position, we can find the "edges" of the data window we are in
    // and find the window size
    
    int window_size = (_rawDataPoint[data_position+1] - _rawDataPoint[data_position]);
    //Serial.print("window_size: ");
    //Serial.print(window_size);

    // diff is the amount (difference) above the bottom of the window
    int diff = (airflowRaw - _rawDataPoint[data_position]);
    //Serial.print("diff: ");
    //Serial.print(diff);
    float percentage_of_window = ((float)diff / (float)window_size);
    //Serial.print("\tpercent: ");
    //Serial.print(percentage_of_window);

    // calculate window size from MPS data points
    float window_size_mps = (_mpsDataPoint[data_position+1] - _mpsDataPoint[data_position]);
    //Serial.print("\twindow_size_mps: ");
    //Serial.print(window_size_mps);

    // add percentage of window_mps to mps.
    airflowMps = _mpsDataPoint[data_position] + (window_size_mps*percentage_of_window);

    return airflowMps;
}

/*************************** READ MILES PER HOUR****************/
/*  Read from sensor, checksum, return mph (0-33ish)     */
float FS3000::readMilesPerHour() {
    return (readMetersPerSecond()*2.2369362912);
}

/*************************** READ DATA **************************/
/*                Read 5 bytes from sensor, put it at a pointer (given as argument)                  */
void FS3000::readData(uint8_t* buffer_in) {
    // Wire.reqeustFrom contains the beginTransmission and endTransmission in it. 
	_i2cPort->requestFrom(FS3000_DEVICE_ADDRESS, 5);  // Request 5 Bytes

  uint8_t i = 0;
	while(_i2cPort->available())
	{
		buffer_in[i] = _i2cPort->read();				// Receive Byte
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

bool FS3000::checksum(uint8_t* data_in, bool debug) {
    uint8_t sum = 0;
    for (int i = 1; i <= 4; i++) {
        sum += uint8_t(data_in[i]);
    }
  
    if(debug)
    {
      for(int i = 0 ; i < 5 ; i++)
      {
        Serial.print(_buff[i], HEX);
        Serial.print(" ");
      }
      Serial.print("\n\rSum of received data bytes                       = ");
      printHexByte(sum);
    }

    sum %= 256;
    //uint8_t calculated_cksum = -sum;
    uint8_t calculated_cksum = (~(sum) + 1);
    uint8_t crcbyte = data_in[0];
    uint8_t overall = sum+crcbyte;

    if(debug) 
    {
      Serial.print("Calculated checksum                              = ");
      printHexByte(calculated_cksum);
      Serial.print("Received checksum byte                           = ");
      printHexByte(crcbyte);
      Serial.print("Sum of received data bytes and received checksum = ");
      printHexByte(overall);
      Serial.println();
    }

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
