/* SFE_BMP180 library example sketch

This sketch shows how to use the SFE_BMP180 library to read the
Bosch BMP180 barometric pressure sensor.
https://www.sparkfun.com/products/11824

Like most pressure sensors, the BMP180 measures absolute pressure.
This is the actual ambient pressure seen by the device, which will
vary with both altitude and weather.

Before taking a pressure reading you must take a temparture reading.
This is done with startTemperature() and getTemperature().
The result is in degrees C.

Once you have a temperature reading, you can take a pressure reading.
This is done with startPressure() and getPressure().
The result is in millibar (mb) aka hectopascals (hPa).

If you'll be monitoring weather patterns, you will probably want to
remove the effects of altitude. This will produce readings that can
be compared to the published pressure readings from other locations.
To do this, use the sealevel() function. You will need to provide
the known altitude at which the pressure was measured.

If you want to measure altitude, you will need to know the pressure
at a baseline altitude. This can be average sealevel pressure, or
a previous pressure reading at your altitude, in which case
subsequent altitude readings will be + or - the initial baseline.
This is done with the altitude() function.

Hardware connections:

- (GND) to GND
+ (VDD) to 3.3V

(WARNING: do not connect + to 5V or the sensor will be damaged!)

You will also need to connect the I2C pins (SCL and SDA) to your
Arduino. The pins are different on different Arduinos:

Any Arduino pins labeled:  SDA  SCL
Uno, Redboard, Pro:        A4   A5
Mega2560, Due:             20   21
Leonardo:                   2    3

Leave the IO (VDDIO) pin unconnected. This pin is for connecting
the BMP180 to systems with lower logic levels such as 1.8V

Have fun! -Your friends at SparkFun.

The SFE_BMP180 library uses floating-point equations developed by the
Weather Station Data Logger project: http://wmrx00.sourceforge.net/

Our example code uses the "beerware" license. You can do anything
you like with this code. No really, anything. If you find it useful,
buy me a beer someday.

V10 Mike Grusin, SparkFun Electronics 10/24/2013
V1.1.2 Updates for Arduino 1.6.4 5/2015
*/
#include <SFE_BMP180.h>
#include <Wire.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "LowPower.h"//Lib Low-Power Version 1.6.0 by Rocket Scream Electronics. Sketch | Bibliothek einbinden | Bibliothek verwalten

#define USE_LOW_POWER_LIB   
#define PORT_ARDUINO_PRO_MINI_LED 13

#define htons(x) ( ((x)<< 8 & 0xFF00) | \
                   ((x)>> 8 & 0x00FF) )
#define ntohs(x) htons(x)

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)

// You will need to create an SFE_BMP180 object, here called "pressure":
SFE_BMP180 pressure;
byte addresses[][6] = {"1Node","2Node"};

//#define ALTITUDE 1655.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters
#define ALTITUDE 300.0 //Altitude Elxleben in meters
#define MAX_BUF_SIZE 255
// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

//Global error codes
enum ErrorCode
{
  NO_ERROR = 1,
  BMP180_INIT_ERROR = 2,
};

void fatalError(ErrorCode errorCode)
{
  for(;;)
  {
    for(unsigned i=0; i < (unsigned)errorCode; i++)
    {
      digitalWrite(PORT_ARDUINO_PRO_MINI_LED, HIGH);
      LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
      digitalWrite(PORT_ARDUINO_PRO_MINI_LED, LOW);
      LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);
    }
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
  }   
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

void initSensor() {
  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.
    Serial.println("BMP180 init fail\n\n");
    //while(1); // Pause forever.
    fatalError( BMP180_INIT_ERROR );
  }
}

void setup()
{
  pinMode(PORT_ARDUINO_PRO_MINI_LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("init begin");
  Serial.flush();
  
  for(unsigned i=0; i < 3; i++)
  {
    digitalWrite(PORT_ARDUINO_PRO_MINI_LED, HIGH);
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
    digitalWrite(PORT_ARDUINO_PRO_MINI_LED, LOW);
    LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF);
  }
   LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

  Serial.println("init sensor");
  Serial.flush();
  initSensor();
  Serial.println("init radio");
  Serial.flush();
  initRadio();
  Serial.println("init end");
  Serial.flush();
}

void sendValue(unsigned long pressure, unsigned long temperature) {
  char buffer[MAX_BUF_SIZE];
  Serial.print("Sending : ");
  Serial.println(pressure);
  //const bool ok = radio.write( &value, sizeof(unsigned long) );
  static const unsigned HEADERSIZE = 4;
  buffer[0] = HEADERSIZE + sizeof(pressure) + sizeof(temperature);//Package size
  buffer[1] = buffer[0];//Package size verifyer
  buffer[2] = 10;//BMP 180 Sensor id
  buffer[3] = 0;//Node id (unsused)

  pressure = htonl(pressure);//convert to network byte   
  temperature = htonl(temperature);
  memcpy(&buffer[HEADERSIZE], &pressure, sizeof(pressure));
  memcpy(&buffer[HEADERSIZE + sizeof(pressure)], &temperature, sizeof(temperature));
  
  const bool ok = radio.write(&buffer[0], MAX_BUF_SIZE);

  //ok kommt manchmal auch ohne Gegenseite ?
  Serial.println(ok ? "Successfull acknowledged." : "Acknowldege failed."); 
  Serial.flush();
  //delay(1000);
}
  
void loop()
{
  char status;
  double T,P,p0,a;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used inather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:
  
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE*3.28084,0);
  Serial.println(" feet");
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P,2);
          Serial.print(" mb, ");
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P,p0);
          Serial.print("computed altitude: ");
          Serial.print(a,0);
          Serial.print(" meters, ");
          Serial.print(a*3.28084,0);
          Serial.println(" feet");
          
          unsigned long pressureMul_1000 = p0*1000.0+.5;
          sendValue(pressureMul_1000, (T*1000.0) /*todo test for pos and neg values */);
       
#ifdef USE_LOW_POWER_LIB 
          Serial.flush();         
          for(unsigned i=0; i<(10L*60L)/8; i++)
          {
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
          }
#else
          delay(10L * 60L * 1000L);  // Pause for n seconds.
#endif
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
        
#ifdef USE_LOW_POWER_LIB 
          Serial.flush();         

          LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
          LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
#else
          delay(5000);  // Pause for n seconds.
#endif
 
}
