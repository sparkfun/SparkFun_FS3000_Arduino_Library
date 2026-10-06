/*!
 * @file sfDevFS3000.h
 *
 * SparkFun Air Velocity Sensor - FS3000 Arduino Library
 *
 * This library facilitates communication with the FS3000 over I<sup>2</sup>C.
 *
 * Want to support open source hardware? Buy a board from SparkFun!
 *
 * This library was originally written by:
 * Pete Lewis
 * SparkFun Electronics
 * August 5th 2021
 *
 * @author SparkFun Electronics
 * @date 2021-2026
 * @copyright Copyright (c) 2021-2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

// include the sparkfun toolkit headers
#include <sfTk/sfToolkit.h>

// Bus interfaces
#include <sfTk/sfTkII2C.h>

/** FS3000 I2C address - note, the FS3000 does not have an adjustable address. */
#define SF_FS3000_DEFAULT_ADDRESS 0x28

/** Number of bytes in a FS3000 reading:
 *  [0] checksum, [1] data high, [2] data low, [3] generic checksum data, [4] generic checksum data
 */
#define SF_FS3000_DATA_LENGTH 5

/** Sensor range - there are two varieties of the FS3000 sensor */
typedef enum
{
    AIRFLOW_RANGE_7_MPS = 0x00,  // FS3000-1005 has a range of 0-7.23 meters per second
    AIRFLOW_RANGE_15_MPS = 0x01, // FS3000-1015 has a range of 0-15 meters per second
    AIRFLOW_RANGE_INVALID
} FS3000_range_t;

/** Value returned by the value-returning methods when an error occurs */
const uint16_t kFS3000ValueError = 0xFFFF;

/** Error returned when the checksum of the data read from the sensor is invalid */
const sfTkError_t kFS3000ErrChecksum = ksfTkErrFail * 0x2001;

/**
 * @class sfDevFS3000
 * @brief Driver class for the FS3000 air velocity sensor.
 *
 * This class provides a platform independent interface to read data from the FS3000 sensor.
 *
 * Usage:
 * - Call begin() to initialize the sensor with an I2C bus.
 * - Call setRange() to match the version of the sensor in use (FS3000-1005 or FS3000-1015).
 * - Use the read methods to obtain raw, meters per second, or miles per hour values.
 *
 * @note The FS3000 has no register map - a reading is a plain read of 5 bytes from the device.
 *       The provided bus must support a read with no register address (devReg == nullptr, regLength == 0).
 */
class sfDevFS3000
{
  public:
    sfDevFS3000() : _theBus{nullptr}, _range{AIRFLOW_RANGE_7_MPS}
    {
    }

    /**
     * @brief Initializes the FS3000 device on the specified bus.
     *
     * If no bus is specified, the method will fail and return an error.
     *
     * @param theBus Pointer to the I2C bus interface (sfTkII2C) to use for communication. Defaults to nullptr.
     * @return sfTkError_t Error code indicating the result of the initialization.
     */
    sfTkError_t begin(sfTkII2C *theBus = nullptr);

    /**
     * @brief Checks if the FS3000 sensor is connected - the device ACKs its address.
     *
     * @return true if the sensor is detected, false otherwise.
     */
    bool isConnected(void);

    /**
     * @brief Sets the range of the sensor, which must match the version of the sensor in use.
     *
     * This also selects the datapoints (from the graphs in the datasheet pages 6 and 7) used to
     * convert from raw values to meters per second.
     *
     * @param range The range to set. Valid values are:
     *      AIRFLOW_RANGE_7_MPS  - FS3000-1005 (0-7.23 m/sec)
     *      AIRFLOW_RANGE_15_MPS - FS3000-1015 (0-15 m/sec)
     * @return sfTkError_t ksfTkErrOk on success, ksfTkErrInvalidParam on an invalid range.
     */
    sfTkError_t setRange(FS3000_range_t range);

    /**
     * @brief Gets the current range setting.
     *
     * @return FS3000_range_t The current range.
     */
    FS3000_range_t range(void)
    {
        return _range;
    }

    /**
     * @brief Reads the raw air velocity value from the sensor and validates the checksum.
     *
     * @param[out] raw The raw value read from the sensor (409-3686)
     * @return sfTkError_t ksfTkErrOk on success, kFS3000ErrChecksum on a checksum failure, or a bus error.
     */
    sfTkError_t readRaw(uint16_t &raw);

    /**
     * @brief Reads the raw air velocity value from the sensor.
     *
     * @return uint16_t The raw value (409-3686), or kFS3000ValueError if an error occurs.
     */
    uint16_t readRaw(void);

    /**
     * @brief Reads the air velocity in meters per second.
     *
     * @param[out] mps The air velocity in m/s: 0-7.23 for the FS3000-1005, 0-15 for the FS3000-1015
     * @return sfTkError_t ksfTkErrOk on success, or an error code.
     */
    sfTkError_t readMetersPerSecond(float &mps);

    /**
     * @brief Reads the air velocity in meters per second.
     *
     * @return float The air velocity in m/s, or kFS3000ValueError if an error occurs.
     */
    float readMetersPerSecond(void);

    /**
     * @brief Reads the air velocity in miles per hour.
     *
     * @param[out] mph The air velocity in mph: 0-16.17 for the FS3000-1005, 0-33.55 for the FS3000-1015
     * @return sfTkError_t ksfTkErrOk on success, or an error code.
     */
    sfTkError_t readMilesPerHour(float &mph);

    /**
     * @brief Reads the air velocity in miles per hour.
     *
     * @return float The air velocity in mph, or kFS3000ValueError if an error occurs.
     */
    float readMilesPerHour(void);

  private:
    /**
     * @brief Reads the 5 data bytes from the sensor
     *
     * @param[out] data Buffer of at least SF_FS3000_DATA_LENGTH bytes
     * @return sfTkError_t Error code indicating the result of the read.
     */
    sfTkError_t readData(uint8_t *data);

    /**
     * @brief Validates the checksum of the data read from the sensor
     *
     * @param data Buffer of SF_FS3000_DATA_LENGTH bytes read from the sensor
     * @return true if the checksum is valid, false otherwise.
     */
    bool checksum(const uint8_t *data);

    /** Pointer to the I2C bus interface used for communication with the FS3000 sensor. */
    sfTkII2C *_theBus;

    /** The current range of the sensor */
    FS3000_range_t _range;
};
