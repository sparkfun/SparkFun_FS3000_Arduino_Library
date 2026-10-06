/*!
 * @file Example01_BasicReadings.ino
 *
 * This example was written by:
 * Pete Lewis
 * SparkFun Electronics
 * August 5th 2021
 *
 * Read values of air velocity from the FS3000 sensor, print them to terminal.
 * Prints raw data, m/s and mph.
 * Note, the response time on the sensor is 125ms.
 *
 * Want to support open source hardware? Buy a board from SparkFun!
 * <br>SparkFun Air Velocity Sensor Breakout - FS3000-1005 (Qwiic) (SEN-18377): https://www.sparkfun.com/products/18377
 * <br>SparkFun Air Velocity Sensor Breakout - FS3000-1015 (Qwiic) (SEN-18768): https://www.sparkfun.com/products/18768
 *
 * Hardware Connections:
 * Use a Qwiic cable to connect from the RedBoard Qwiic to the FS3000 breakout (QWIIC).
 * You can also choose to wire up the connections using the header pins like so:
 *
 *   ARDUINO --> FS3000
 *   SDA (A4) --> SDA
 *   SCL (A5) --> SCL
 *   3.3V --> 3.3V
 *   GND --> GND
 *
 * Please see LICENSE.md for the license information
 *
 */

#include <SparkFun_FS3000_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_FS3000

SparkFunFS3000 mySensor; // Create a FS3000 object

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Example 1 - Reading values from the FS3000"));

    Wire.begin();

    // Begin the FS3000 using the Wire I2C port
    // .begin will return true on success, or false on failure to communicate
    if (mySensor.begin() == false)
    {
        Serial.println(F("The sensor did not respond. Please check wiring. Freezing..."));
        while (1)
            ;
    }

    // Set the range to match which version of the sensor you are using.
    // FS3000-1005 (0-7.23 m/sec) --->>>  AIRFLOW_RANGE_7_MPS
    // FS3000-1015 (0-15 m/sec)   --->>>  AIRFLOW_RANGE_15_MPS
    mySensor.setRange(AIRFLOW_RANGE_7_MPS);
    // mySensor.setRange(AIRFLOW_RANGE_15_MPS);

    Serial.println(F("Sensor is connected properly."));
}

void loop()
{
    Serial.print(F("FS3000 Readings \tRaw: "));
    Serial.print(mySensor.readRaw()); // note, this returns an int from 0-3686

    Serial.print(F("\tm/s: "));
    Serial.print(mySensor.readMetersPerSecond()); // note, this returns a float from 0-7.23 for the FS3000-1005, and 0-15 for the FS3000-1015

    Serial.print(F("\tmph: "));
    Serial.println(mySensor.readMilesPerHour()); // note, this returns a float from 0-16.17 for the FS3000-1005, and 0-33.55 for the FS3000-1015

    delay(1000); // note, response time on the sensor is 125ms
}
