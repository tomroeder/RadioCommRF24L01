/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 03/17/2013 : Charles-Henri Hallard (http://hallard.me)
              Modified to use with Arduipi board http://hallard.me/arduipi
						  Changed to use modified bcm2835 and RF24 library
TMRh20 2014 - Updated to work with optimized RF24 Arduino library

 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <RF24/RF24.h>


#define LOAD_SIZE_1_IDX 0
#define LOAD_SIZE_2_IDX 1
#define SENSOR_TYPE_IDX 2
#define SENSOR_ID_IDX   3

#define SENSOR_TYPE_BMP_180_PRESSURE_AND_TEMPERATURE 10

using namespace std;

// CE Pin, CSN Pin, SPI Speed

//RPi Revision 1 mit CE an Pin 22
RF24 radio(RPI_GPIO_P1_22 /* je nachdem welcher GPIO verdrahtet wurde */, RPI_GPIO_P1_24 /* identisch zu RPi Rev 2 */, BCM2835_SPI_SPEED_8MHZ);
//RF24 radio(RPI_GPIO_P1_22 /* je nachdem welcher GPIO verdrahtet wurde */, RPI_GPIO_P1_24 /* identisch zu RPi Rev 2 */, BCM2835_SPI_SPEED_1MHZ);

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

static const uint8_t MAX_BUF_SIZE = 255;

void parseSensorTypeBmp180Data(const uint8_t * buf, uint8_t bufSize)
{
  printf("Receiced %d bytes from \'BMP 180\' air pressure and temperature sensor.\n", bufSize);
  uint32_t pressure;
  uint32_t temperature;

  if (sizeof(pressure)+sizeof(temperature) != bufSize)
  {
    printf("Data size mismatch.\n");
    return;
  }
  memcpy(&pressure, buf, sizeof(pressure));
  memcpy(&temperature, buf+sizeof(pressure), sizeof(temperature));

  pressure = ntohl(pressure);
  temperature = ntohl(temperature);

  printf("Received pressure    = %.2f mb.\n", (pressure / 1000.) );
  printf("Received temperature = %.2f °C.\n", (temperature / 1000.) );
}

int main(int argc, char** argv){

  printf("RF24/examples/pong_back\n");

  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);//15, 15 is maximum
  radio.setPALevel(RF24_PA_MIN);//MIN, LOW, HIGH, MAX, kleiner bei instabiler Spannungsversorgung, dafür weniger Reichweite
  // Dump the configuration of the rf unit for debugging
  radio.printDetails();

  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();


  while (1)
  {
    if ( radio.available() )
    {
#if 0
	// Dump the payloads until we've gotten everything
	unsigned long data;
	//while(radio.available())
	// Fetch the payload, and see if this was the last one.
	radio.read( &data, sizeof(unsigned long) );
	// Spew it
	printf("Got payload(%d) %lu...\n",sizeof(unsigned long), data);
	printf("relative (sea level) pressure = %.2f mb.\n", data / 1000.);

#else
        uint8_t buf[MAX_BUF_SIZE];
	memset(&buf[0], 0, MAX_BUF_SIZE);
	radio.read(&buf[0], MAX_BUF_SIZE);

	const uint8_t size1 = buf[LOAD_SIZE_1_IDX];
	const uint8_t size2 = buf[LOAD_SIZE_2_IDX];
	if (size1!=size2)
	{
          printf("Invalid header size %d!=%d.\n", size1, size2);
	  break;
	}
	else
	{
	  printf("Valid header size : Expecting %d bytes package load.\n", size1);
	}


	const uint8_t sensorType = buf[SENSOR_TYPE_IDX];
	const uint8_t sensorId = buf[SENSOR_ID_IDX];

	static const uint8_t headerSize = 4;
	const uint8_t sensorDataSize = size1 - headerSize;//4 Byte Headersize
	switch(sensorType)
	{
		case SENSOR_TYPE_BMP_180_PRESSURE_AND_TEMPERATURE:
		  parseSensorTypeBmp180Data(&buf[headerSize], sensorDataSize);
		  break;
		default:
		  printf("Unknown sensor type.\n");
		  break;
	}
#endif
	//delay(100);//nicht besser
	delay(925); //Delay after payload responded to, minimize RPi CPU time
    }

  } // forever loop
  return 0;
}



