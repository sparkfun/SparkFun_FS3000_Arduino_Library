/*!
 * @file Example02_ErrorChecking.ino
 *
 * Read the air velocity from the FS3000 sensor, checking the error code returned for each reading.
 *
 * Each read method has a version that returns an error code (sfTkError_t) and passes the value back
 * through a reference parameter. This allows a sketch to detect communication errors and data that
 * fails the sensor's checksum.
 *
 * Want to support open source hardware? Buy a board from SparkFun!
 * <br>SparkFun Air Velocity Sensor Breakout - FS3000-1005 (Qwiic) (SEN-18377): https://www.sparkfun.com/products/18377
 * <br>SparkFun Air Velocity Sensor Breakout - FS3000-1015 (Qwiic) (SEN-18768): https://www.sparkfun.com/products/18768
 *
 * Please see LICENSE.md for the license information
 *
 */

#include <SparkFun_FS3000_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_FS3000

SparkFunFS3000 mySensor; // Create a FS3000 object

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Example 2 - Reading values from the FS3000 with error checking"));

    Wire.begin();

    if (mySensor.begin() == false)
    {
        Serial.println(F("The sensor did not respond. Please check wiring. Freezing..."));
        while (1)
            ;
    }

    // Set the range to match which version of the sensor you are using.
    // FS3000-1005 (0-7.23 m/sec) --->>>  AIRFLOW_RANGE_7_MPS
    // FS3000-1015 (0-15 m/sec)   --->>>  AIRFLOW_RANGE_15_MPS
    if (mySensor.setRange(AIRFLOW_RANGE_7_MPS) != ksfTkErrOk)
        Serial.println(F("Invalid range setting"));

    Serial.println(F("Sensor is connected properly."));
}

void loop()
{
    float mps;
    sfTkError_t rc = mySensor.readMetersPerSecond(mps);

    if (rc == ksfTkErrOk)
    {
        Serial.print(F("FS3000 m/s: "));
        Serial.println(mps);
    }
    else if (rc == kFS3000ErrChecksum)
        Serial.println(F("Checksum error - data from the sensor is invalid"));
    else
    {
        Serial.print(F("Error reading the sensor: "));
        Serial.println(rc);
    }

    delay(1000); // note, response time on the sensor is 125ms
}
