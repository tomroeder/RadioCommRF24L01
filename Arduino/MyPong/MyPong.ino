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
  initRadio();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
}

void initRadio() {
  // Setup and configure rf radio
  radio.begin();                          // Start up the radio
  delay(100);
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.setPALevel(RF24_PA_MIN);//MIN, LOW, HIGH, MAX, kleiner setzen bei Problemen mit Spannungsversorgung
  radio.openReadingPipe(1,addresses[0]);  
  radio.startListening();                 // Start listening
}

void loop(void){
  if( radio.available()){//blockiert nicht !
    unsigned long got_time;                                       // Variable for the received timestamp
    radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
    printf("Received %lu \n\r", got_time);
  }
  delay(100);//deutlich besser als vorher
}
