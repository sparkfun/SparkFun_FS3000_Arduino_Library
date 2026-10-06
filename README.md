# SparkFun Air Velocity Sensor - FS3000 Arduino Library

SparkFun Air Velocity Sensor Breakout - FS3000 (Qwiic)

![License](https://img.shields.io/github/license/sparkfun/SparkFun_FS3000_Arduino_Library)
![Release](https://img.shields.io/github/v/release/sparkfun/SparkFun_FS3000_Arduino_Library)
![Release Date](https://img.shields.io/github/release-date/sparkfun/SparkFun_FS3000_Arduino_Library)
![GitHub issues](https://img.shields.io/github/issues/sparkfun/SparkFun_FS3000_Arduino_Library)

This library provides access to the Renesas FS3000 air velocity sensor through an I2C connection using the SparkFun Qwiic connectors and cables. The FS3000 is a surface-mount type air velocity module utilizing a MEMS thermopile-based sensor, with a digital output at 12-bit resolution.

> [!NOTE]
> Version 2.0 and above of this library is built on the [SparkFun Toolkit](https://github.com/sparkfun/SparkFun_Toolkit), which must also be installed.
>
> Sketches written for version 1.x continue to compile. The main class is now `SparkFunFS3000` - the `FS3000` class is still available, but is deprecated. Readings that fail the sensor's checksum now return an error value (`kFS3000ValueError`) instead of the unvalidated data. See the online documentation and examples for further information.

### Supported Products

There are two versions of this sensor with different upper ranges (1005/1015). This library is intended for use with the following SparkFun Products - available at [www.sparkfun.com](https://www.sparkfun.com).

| Product | Description|
|--|--|
|[SparkFun Air Velocity Sensor Breakout - FS3000-1005 (Qwiic)](https://www.sparkfun.com/products/18377) | The 1005 version of the FS3000, which can sense 0-7.23m/s (0-16.17mph).|
|[SparkFun Air Velocity Sensor Breakout - FS3000-1015 (Qwiic)](https://www.sparkfun.com/products/18768) | The 1015 version of the FS3000, which can sense 0-15m/s (0-33.6mph).|

## Documentation

|Reference | Description |
|---|---|
|[Product Repository](https://github.com/sparkfun/SparkFun_Air_Velocity_Sensor_FS3000_Qwiic)| Hardware GitHub Repository|
|[SparkFun Air Velocity Sensor - FS3000 Arduino Library](https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library)| Arduino Library - GitHub Repository|
|[SparkFun Toolkit](https://github.com/sparkfun/SparkFun_Toolkit)| The SparkFun Toolkit library this library depends on|
|[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)| Basic information on how to install an Arduino library|

## Examples

The following examples are provided with the library

| Example | Description |
|---|---|
|[Basic Readings](https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library/blob/main/examples/Example01_BasicReadings/Example01_BasicReadings.ino)| Read the air velocity from the sensor - prints raw data, m/s and mph.|
|[Error Checking](https://github.com/sparkfun/SparkFun_FS3000_Arduino_Library/blob/main/examples/Example02_ErrorChecking/Example02_ErrorChecking.ino)| Read the air velocity from the sensor, checking the error code returned for each reading.|

## License Information

This product is ***open source***!

This product is licensed using the [MIT Open Source License](https://opensource.org/license/mit).
