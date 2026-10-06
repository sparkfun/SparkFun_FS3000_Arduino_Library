/******************************************************************************
 * @file SparkFun_FS3000_Arduino_Library.h
 * @brief SparkFun FS3000 Library header file
 *
 * This file implements the SparkFunFS3000 class
 * for use with the SparkFun Air Velocity Sensor - FS3000 qwiic breakout board
 *
 * @author SparkFun Electronics
 * @date 2021-2026
 * @version 2.0.0
 * @copyright (c) 2021-2026 SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 ******************************************************************************/

#pragma once

// helps to keep the Toolkit header before the tk calls
// clang-format off
#include <SparkFun_Toolkit.h>
#include "sfTk/sfDevFS3000.h"
// clang-format on

// For backwards compatibility with version 1.x of this library
#define FS3000_DEVICE_ADDRESS SF_FS3000_DEFAULT_ADDRESS
#define FS3000_TO_READ SF_FS3000_DATA_LENGTH

/**
 * @brief Arduino I2C bus that supports reads with no register address.
 *
 * The FS3000 has no register map - a reading is a plain I2C read of 5 bytes. The toolkit
 * Arduino I2C bus requires a register address for reads, so this subclass adds support for
 * reads where no register is provided (devReg == nullptr or regLength == 0).
 */
class sfTkArdI2CFS3000 : public sfTkArdI2C
{
  public:
    /**
     * @brief Reads data from the device - if no register address is provided, a plain read is performed.
     *
     * @param devReg Pointer to the register address to read from - nullptr for no register.
     * @param regLength Length of the register address - 0 for no register.
     * @param data Pointer to the buffer where the read data will be stored.
     * @param numBytes Number of bytes to read.
     * @param readBytes Reference to a variable where the number of bytes actually read will be stored.
     * @param read_delay After sending the address, delay in milliseconds before reading the data
     * @return sfTkError_t Error code indicating the success or failure of the read operation.
     */
    sfTkError_t readRegister(uint8_t *devReg, size_t regLength, uint8_t *data, size_t numBytes, size_t &readBytes,
                             uint32_t read_delay = 0) override
    {
        if (devReg != nullptr && regLength > 0)
            return sfTkArdI2C::readRegister(devReg, regLength, data, numBytes, readBytes, read_delay);

        readBytes = 0;

        if (!_i2cPort)
            return ksfTkErrBusNotInit;

        if (!data)
            return ksfTkErrBusNullBuffer;

        size_t nReturned = _i2cPort->requestFrom((int)address(), (int)numBytes);

        for (; readBytes < nReturned && readBytes < numBytes; readBytes++)
            *data++ = _i2cPort->read();

        return readBytes == numBytes ? ksfTkErrOk : ksfTkErrBusUnderRead;
    }

    // Make sure the other readRegister overloads remain visible on this object
    using sfTkArdI2C::readRegister;
};

/**
 * @brief Class for interfacing with the FS3000 sensor using I2C communication
 *
 * This class provides methods to initialize and communicate with the FS3000 sensor
 * over an I2C bus. It inherits from the sfDevFS3000 class and uses the SparkFun
 * Toolkit for I2C communication.
 *
 * @see sfDevFS3000
 */
class SparkFunFS3000 : public sfDevFS3000
{
  public:
    /**
     * @brief Begins the Device with I2C as the communication bus
     *
     * This method initializes the I2C bus and sets up communication with the FS3000 sensor.
     *
     * @note The FS3000 does not have an adjustable address.
     *
     * @param wirePort Wire port to use for I2C communication
     * @return True if successful, false otherwise
     */
    bool begin(TwoWire &wirePort = Wire)
    {
        // Setup Arduino I2C bus
        _theI2CBus.init(wirePort, SF_FS3000_DEFAULT_ADDRESS);

        // Begin the sensor
        sfTkError_t rc = sfDevFS3000::begin(&_theI2CBus);

        return rc == ksfTkErrOk ? isConnected() : false;
    }

  private:
    sfTkArdI2CFS3000 _theI2CBus;
};

// for backwards compatibility
/**
 * @brief Deprecated class for interfacing with the FS3000 sensor - supports version 1.x of this library
 *
 * @deprecated This class is deprecated for version 2.0 of this library. Use SparkFunFS3000 instead.
 */
class FS3000 : public SparkFunFS3000
{
};
