# CryptoAuthLib
Atmel CryptoAuthLib + my own HAL implementations

See [./cryptoauthlib](https://github.com/Petezah/CryptoAuthLib/tree/master/cryptoauthlib) for Atmel CryptoAuthLib 20161123 and notes about that particular version.  The library itself is unchanged from Atmel's distribution.  Only my HAL implementations are unique in this repository.

## Contents

* atec_hal_raspi_i2c.cpp - A HAL implementation for the I2C bus on the Raspberry Pi
  * Details: Uses the [gnublin API](https://github.com/embeddedprojects/gnublin-api) for I2C and [Wiring Pi](http://wiringpi.com/) for a couple of delay functions it implements.  Wiring Pi's I2C implementation just did not work for me, at least the way CryptoAuthLib is expecting to interface with it anyway.  Gnublin's, on the other hand, is more flexible and seems a bit more robust.
* main.cpp - A simple test to exercise some functions in the CryptoAuthLib API
* ATECC508aTest.* - A Visual Studio 2017 Linux project to test out the library
  * All the code actually builds on the Raspberry Pi for this project; it just needs a Makefile to build there stand-alone.  It's just that it was expedient to test the library this way.
  
## Todo

* Write a Makefile to do the test build stand-alone on the Raspberry Pi, etc
