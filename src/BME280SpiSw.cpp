/*
BME280SpiI2C.cpp
This code records data from the BME280Spi sensor and provides an API.
This file is part of the Arduino BME280Spi library.
Copyright (C) 2016  Tyler Glenn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Written: Dec 30 2015.
Last Updated: Jan 1 2016. - Happy New year!

This header must be included in any derived code or copies of the code.

Based on the data sheet provided by Bosch for the BME280Spi environmental sensor,
calibration code based on algorithms providedBosch, some unit conversations courtesy
of www.endmemo.com, altitude equation courtesy of NOAA, and dew point equation
courtesy of Brian McNoldy at http://andrew.rsmas.miami.edu.
 */


/* ==== Includes ==== */
#include "Arduino.h"
#include <SPI.h>
#include "BME280Spi.h"
#include <Adafruit_BMP280.h>
/* ====  END Includes ==== */

/* ==== Methods ==== */

bool BME280Spi::Initialize() {
  WriteRegister(CTRL_HUM_ADDR, controlHumidity);
  WriteRegister(CTRL_MEAS_ADDR, controlMeasure);
  WriteRegister(CONFIG_ADDR, config);
  return ReadTrim();
}


uint8_t BME280Spi::SpiTransferSw(uint8_t data)
{
  uint8_t resp = 0;
  for (int bit = 7; bit >= 0; --bit) {
    resp <<= 1;
    digitalWrite(sckPin, LOW);
    digitalWrite(mosiPin, data & (1 << bit));
    digitalWrite(sckPin, HIGH);
    resp |= digitalRead(misoPin))
  }
  return resp;
}

bool BME280Spi::ReadAddr(uint8_t addr, uint8_t array[], uint8_t len)
{
  if(mosiPin == -1)
  {
      ReadAddrHwSpi(addr, &array[0], len);
  }
  else
  {
      ReadAddrSwSpi(addr, &array[0], len);
  }
}

bool BME280Spi::ReadAddrHwSpi(uint8_t addr, uint8_t array[], uint8_t len) {

  SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

  // bme280 uses the msb to select read and write
  // combine the addr with the read/write bit
  uint8_t readAddr = addr |  BME280_SPI_READ;

  //select the device
  digitalWrite(csPin, LOW);
  // transfer the addr
  SPI.transfer(readAddr);

  // read the data
  for(int i = 0; i < len; ++i)
  {
    // transfer 0x00 to get the data
    array[i] = SPI.transfer(0);
  }

  // de-select the device
  digitalWrite(csPin, HIGH);

  SPI.endTransaction();

  return true;
}
bool BME280Spi::ReadAddrSwSpi(uint8_t addr, uint8_t array[], uint8_t len) {

  // bme280 uses the msb to select read and write
  // combine the addr with the read/write bit
  uint8_t readAddr = addr |  BME280_SPI_READ;

  //select the device
  digitalWrite(csPin, LOW);
  // transfer the addr
  SPI.transfer(readAddr);

  // read the data
  for(int i = 0; i < len; ++i)
  {
    // transfer 0x00 to get the data
    array[i] = SpiTransferSw(0);
  }

  // de-select the device
  digitalWrite(csPin, HIGH);

  return true;
}


void BME280Spi::WriteRegister(uint8_t addr, uint8_t data)
{
  if(mosiPin == -1)
  {
      WriteRegisterHwSpi(addr, &array[0], len);
  }
  else
  {
      WriteRegisterSwSpi(addr, &array[0], len);
  }
}

void BME280Spi::WriteRegisterSwSpi(uint8_t addr, uint8_t data)
{
  // bme280 uses the msb to select read and write
  // combine the addr with the read/write bit
  uint8_t writeAddr = addr & ~0x80;

  // select the device
  digitalWrite(csPin, LOW);

  // transfer the addr and then the data to spi device
  SPI.transfer(writeAddr);
  SPI.transfer(data);

  // de-select the device
  digitalWrite(csPin, HIGH);

  SPI.endTransaction();
}

void BME280Spi::WriteRegisterHwSpi(uint8_t addr, uint8_t data)
{
  SPI.beginTransaction(SPISettings(500000,MSBFIRST,SPI_MODE0));

  // bme280 uses the msb to select read and write
  // combine the addr with the read/write bit
  uint8_t writeAddr = addr & ~0x80;

  // select the device
  digitalWrite(csPin, LOW);

  // transfer the addr and then the data to spi device
  SPI.transfer(writeAddr);
  SPI.transfer(data);

  // de-select the device
  digitalWrite(csPin, HIGH);

  SPI.endTransaction();
}

bool BME280Spi::ReadTrim()
{
  uint8_t ord(0);

  // Temp dig.
  ReadAddr(TEMP_DIG_ADDR, &dig[ord], 6);
  ord += 6;

  // Pressure dig.
  ReadAddr(PRESS_DIG_ADDR, &dig[ord], 18);
  ord += 18;

  // Humidity dig1.
  ReadAddr(HUM_DIG_ADDR1, &dig[ord], 1);
  ord += 1;

  // Humidity dig2.
  ReadAddr(HUM_DIG_ADDR2, &dig[ord], 7);
  ord += 7;


  // should always return true
  return ord == 32;
}

bool BME280Spi::ReadData(int32_t data[8]){

  uint8_t temp[8];
  uint8_t ord(0);

  ReadAddr(PRESS_ADDR, &temp[ord], 3);
  ord += 3;

  ReadAddr(TEMP_ADDR, &temp[ord], 3);
  ord += 3;

  ReadAddr(HUM_ADDR, &temp[ord], 2);
  ord += 2;


  for(int i = 0; i < 8; ++i)
  {
    data[i] = temp[i];
  }

  return true;
}


BME280Spi::BME280Spi(uint8_t spiCsPin, uint8_t tosr, uint8_t hosr, uint8_t posr, uint8_t mode, uint8_t st, uint8_t filter):
    BME280(tosr, hosr, posr, mode, st, filter, false), csPin(spiCsPin), mosiPin(-1), misoPin(-1), sckPin(-1);
{
}

BME280Spi::BME280Spi(uint8_t spiCsPin, uint8_t spiMosiPin, uint8_t spiMisoPin, uint8_t spiSckPin, uint8_t tosr = 0x1,
  uint8_t hosr = 0x1, uint8_t posr = 0x1, uint8_t mode = 0x3, uint8_t st = 0x5, uint8_t filter = 0x0):
  BME280(tosr, hosr, posr, mode, st, filter, false),
  csPin(spiCsPin), mosiPin(spiMosiPin), misoPin(spiMisoPin), sckPin(spiSckPin)
  {
  }


bool BME280Spi::begin(){

  digitalWrite(csPin, HIGH);
  pinMode(csPin, OUTPUT);

  if(mosiPin == -1)
  {
      // Hardware spi
      SPI.begin();
  }
  else
  {
      // Software spi
      pinMode(sckPin, OUTPUT);
      pinMode(mosiPin, OUTPUT);
      pinMode(misoPin, INPUT);
  }


  uint8_t id[1];
  ReadAddr(ID_ADDR, &id[0], 1);

  if (id[0] != BME_ID && id[0] != BMP_ID)
  {
      return false;
  }

  return Initialize();
}

/* ==== END Methods ==== */