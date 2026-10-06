/*!
 * @file sfDevFS3000.cpp
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

#include "sfDevFS3000.h"

/**
 * @brief Conversion datapoints from raw values to meters per second.
 *
 * These are from the graphs in the datasheet, pages 6 and 7. The output curve is not a straight
 * line, so a reading is converted by finding the two datapoints it falls between and treating
 * the curve as a straight line between them.
 */
static const float kFS3000MpsDataPoint_7_mps[] = {0, 1.07, 2.01, 3.00, 3.97, 4.96, 5.98, 6.99, 7.23};
static const uint16_t kFS3000RawDataPoint_7_mps[] = {409, 915, 1522, 2066, 2523, 2908, 3256, 3572, 3686};

static const float kFS3000MpsDataPoint_15_mps[] = {0,    2.00, 3.00,  4.00,  5.00,  6.00, 7.00,
                                                   8.00, 9.00, 10.00, 11.00, 13.00, 15.00};
static const uint16_t kFS3000RawDataPoint_15_mps[] = {409,  1203, 1597, 1908, 2187, 2400, 2629,
                                                      2801, 3006, 3178, 3309, 3563, 3686};

/** Conversion from meters per second to miles per hour */
static const float kFS3000MpsToMph = 2.2369362912;

//--------------------------------------------------------------------------------------------------
// Begin the FS3000 device. Requires a bus object to communicate with the device.
//
sfTkError_t sfDevFS3000::begin(sfTkII2C *theBus)
{
    // Nullptr check
    if (theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Set bus pointer
    _theBus = theBus;

    return ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
// Check if the FS3000 is connected - the device ACKs its address
//
bool sfDevFS3000::isConnected(void)
{
    if (_theBus == nullptr)
        return false;

    return _theBus->ping() == ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
// Set the range of the sensor - this must match the version of the sensor in use.
//
sfTkError_t sfDevFS3000::setRange(FS3000_range_t range)
{
    if (range >= AIRFLOW_RANGE_INVALID)
        return ksfTkErrInvalidParam;

    _range = range;

    return ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
// Read the 5 data bytes from the sensor. The FS3000 has no registers, so this is a plain read.
//
sfTkError_t sfDevFS3000::readData(uint8_t *data)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    size_t nRead;
    sfTkError_t rc = _theBus->readRegister(nullptr, 0, data, SF_FS3000_DATA_LENGTH, nRead);

    return (rc == ksfTkErrOk && nRead != SF_FS3000_DATA_LENGTH) ? ksfTkErrBusUnderRead : rc;
}

//--------------------------------------------------------------------------------------------------
// Check that the checksum is correct and the data is good.
//
// The entire response from the FS3000 is 5 bytes:
//   [0] checksum
//   [1] data high
//   [2] data low
//   [3] generic checksum data
//   [4] generic checksum data
//
// The sum of all 5 bytes (mod 256) is zero if the data is good.
//
bool sfDevFS3000::checksum(const uint8_t *data)
{
    uint8_t sum = 0;
    for (int i = 0; i < SF_FS3000_DATA_LENGTH; i++)
        sum += data[i];

    return sum == 0x00;
}

//----------------------------------------------------------------------------------------------------
// Read the raw value from the sensor, validating the checksum - also return an error code
//
sfTkError_t sfDevFS3000::readRaw(uint16_t &raw)
{
    raw = 0;

    uint8_t data[SF_FS3000_DATA_LENGTH];
    sfTkError_t rc = readData(data);

    if (rc != ksfTkErrOk)
        return rc;

    if (!checksum(data))
        return kFS3000ErrChecksum;

    // The flow data is a 12-bit integer. Only the least significant four bits in the high byte are valid.
    raw = ((uint16_t)(data[1] & 0x0F) << 8) | data[2];

    return ksfTkErrOk;
}

//----------------------------------------------------------------------------------------------------
// Read the raw value from the sensor
//
uint16_t sfDevFS3000::readRaw(void)
{
    uint16_t raw;
    sfTkError_t rc = readRaw(raw);

    return rc == ksfTkErrOk ? raw : kFS3000ValueError;
}

//----------------------------------------------------------------------------------------------------
// Read the air velocity in meters per second - also return an error code
//
sfTkError_t sfDevFS3000::readMetersPerSecond(float &mps)
{
    mps = 0.0f;

    uint16_t airflowRaw;
    sfTkError_t rc = readRaw(airflowRaw);

    if (rc != ksfTkErrOk)
        return rc;

    // Select the datapoints for the current range - note the datasheet graphs have a different
    // number of datapoints for each range.
    const float *mpsDataPoint = kFS3000MpsDataPoint_7_mps;
    const uint16_t *rawDataPoint = kFS3000RawDataPoint_7_mps;
    uint8_t dataPointsNum = sizeof(kFS3000RawDataPoint_7_mps) / sizeof(kFS3000RawDataPoint_7_mps[0]);

    if (_range == AIRFLOW_RANGE_15_MPS)
    {
        mpsDataPoint = kFS3000MpsDataPoint_15_mps;
        rawDataPoint = kFS3000RawDataPoint_15_mps;
        dataPointsNum = sizeof(kFS3000RawDataPoint_15_mps) / sizeof(kFS3000RawDataPoint_15_mps[0]);
    }

    // At or below the minimum, report 0. At or above the maximum, report the max of the range.
    if (airflowRaw <= rawDataPoint[0])
        return ksfTkErrOk;

    if (airflowRaw >= rawDataPoint[dataPointsNum - 1])
    {
        mps = mpsDataPoint[dataPointsNum - 1];
        return ksfTkErrOk;
    }

    // Find the window (between two datapoints) our raw reading is in
    uint8_t dataPosition = 0;
    for (uint8_t i = 0; i < dataPointsNum; i++)
    {
        if (airflowRaw > rawDataPoint[i])
            dataPosition = i;
    }

    // Find what percentage of the raw window we are at, and apply it to the m/s window
    float windowSize = (float)(rawDataPoint[dataPosition + 1] - rawDataPoint[dataPosition]);
    float diff = (float)(airflowRaw - rawDataPoint[dataPosition]);
    float windowSizeMps = mpsDataPoint[dataPosition + 1] - mpsDataPoint[dataPosition];

    mps = mpsDataPoint[dataPosition] + (windowSizeMps * (diff / windowSize));

    return ksfTkErrOk;
}

//----------------------------------------------------------------------------------------------------
// Read the air velocity in meters per second
//
float sfDevFS3000::readMetersPerSecond(void)
{
    float mps;
    sfTkError_t rc = readMetersPerSecond(mps);

    return rc == ksfTkErrOk ? mps : kFS3000ValueError;
}

//----------------------------------------------------------------------------------------------------
// Read the air velocity in miles per hour - also return an error code
//
sfTkError_t sfDevFS3000::readMilesPerHour(float &mph)
{
    sfTkError_t rc = readMetersPerSecond(mph);

    if (rc == ksfTkErrOk)
        mph *= kFS3000MpsToMph;

    return rc;
}

//----------------------------------------------------------------------------------------------------
// Read the air velocity in miles per hour
//
float sfDevFS3000::readMilesPerHour(void)
{
    float mph;
    sfTkError_t rc = readMilesPerHour(mph);

    return rc == ksfTkErrOk ? mph : kFS3000ValueError;
}
