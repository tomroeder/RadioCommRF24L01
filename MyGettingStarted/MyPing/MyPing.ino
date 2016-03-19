/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
//2014 - TMRh20 - Updated along with Optimized RF24 Library fork
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

byte addresses[][6] = {"1Node","2Node"};

void setup() {
  Serial.begin(57600);
  printf_begin();
  printf("\n\My Ping/\n\r");
  initRadio();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
}

void initRadio() {
  // Setup and configure rf radio
  radio.begin();                          // Start up the radio
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setPALevel(RF24_PA_MIN);//MIN, LOW, HIGH, MAX, kleiner setzen bei Problemen mit Spannungsversorgung
 
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);  
}

void loop(void){
    unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
    printf("Sending :\"%lu\".\r", time);
    const bool ok = radio.write( &time, sizeof(unsigned long) );
    //ok kommt manchmal auch ohne Gegenseite ?
    printf(ok ? "successfull acknowledged.\n\r" : "acknowldedge failed.\n\r"); 
    delay(1000);
}
