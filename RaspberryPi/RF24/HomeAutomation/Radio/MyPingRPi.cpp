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

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

int main(int argc, char** argv){

  printf("RF24/examples/ping out/\n");
  // Setup and configure rf radio
  radio.begin();
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);//15, 15 is maximum
  radio.setPALevel(RF24_PA_MIN);//MIN, LOW, HIGH, MAX, kleiner bei instabiler Spannungsversorgung, dafür weniger Reichweite
  // Dump the configuration of the rf unit for debugging
  radio.printDetails();

  radio.openWritingPipe(pipes[0]);

  while (1) {
    // First, stop listening so we can talk.
    radio.stopListening();

    unsigned long time = millis();
    printf("Now sending \'%lu\' ... ", time);
    const bool ok = radio.write( &time, sizeof(unsigned long) );

    printf(ok ? "acknowledged.\n" : "acknowledge failed.\n");
    sleep(1);
  }
  return 0;
}