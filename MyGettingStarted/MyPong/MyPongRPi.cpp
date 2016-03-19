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
#include <RF24/RF24.h>

using namespace std;

// CE Pin, CSN Pin, SPI Speed

//RPi Revision 1 mit CE an Pin 22
RF24 radio(RPI_GPIO_P1_22 /* je nachdem welcher GPIO verdrahtet wurde */, RPI_GPIO_P1_24 /* identisch zu RPi Rev 2 */, BCM2835_SPI_SPEED_8MHZ);
//RF24 radio(RPI_GPIO_P1_22 /* je nachdem welcher GPIO verdrahtet wurde */, RPI_GPIO_P1_24 /* identisch zu RPi Rev 2 */, BCM2835_SPI_SPEED_1MHZ);

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

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
	// Dump the payloads until we've gotten everything
	unsigned long got_time;
	//while(radio.available())
	// Fetch the payload, and see if this was the last one.
	radio.read( &got_time, sizeof(unsigned long) );
	// Spew it
	printf("Got payload(%d) %lu...\n",sizeof(unsigned long), got_time);

	//delay(100);//nicht besser
	delay(925); //Delay after payload responded to, minimize RPi CPU time
    }

  } // forever loop
  return 0;
}

