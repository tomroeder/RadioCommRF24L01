#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <iomanip>

#include <RF24/RF24.h>
#include <sqlite3.h>

// -- Daemon --
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
// --

#define LOAD_SIZE_1_IDX 0
#define LOAD_SIZE_2_IDX 1
#define SENSOR_TYPE_IDX 2
#define SENSOR_ID_IDX   3

#define SENSOR_TYPE_BMP_180_PRESSURE_AND_TEMPERATURE 10

#define SQLITE_FILE_NAME "/home/pi/Data/Rails/blog/db/development.sqlite3"

using namespace std;

// CE Pin, CSN Pin, SPI Speed

//RPi Revision 1 mit CE an Pin 22
RF24 radio(RPI_GPIO_P1_22 /* je nachdem welcher GPIO verdrahtet wurde */, RPI_GPIO_P1_24 /* identisch zu RPi Rev 2 */, BCM2835_SPI_SPEED_8MHZ);

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};

static const uint8_t MAX_BUF_SIZE = 255;

//! Callback for SQLiteDb
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

/*!
* SQLite 3 wrapper
**/
class SQLiteDb
{
public:

  SQLiteDb() : m_db(NULL) {}

  ~SQLiteDb() { if (NULL != m_db) { sqlite3_close(m_db); } }
  
  int Open(const char * fileName) 
  {
    const int ret = sqlite3_open(fileName, &m_db);
    if (ret) { sqlite3_close(m_db);  m_db=NULL; }
    return ret;
  }

  int Execute(const char * cmd)
  {
    char * errMsg = NULL;
    if (NULL == m_db) { return -1; }
    const int ret = sqlite3_exec(m_db, cmd, callback, 0, &errMsg);
    if ( ret != SQLITE_OK)
    {
      fprintf(stderr, "SQL error: %s.\n", errMsg);
      sqlite3_free(errMsg);
    }
    return ret;
  }

protected:
private:
  sqlite3 * m_db;
};

std::string GetCurrentTime()
{
  time_t t;
  struct tm *ts;

  t = time(NULL);
  ts = localtime(&t);

  stringstream ss;

  ss  << ts->tm_year + 1900;
  ss << "-" << setfill('0') << setw(2) << ts->tm_mon + 1;
  ss << "-" << setfill('0') << setw(2) << ts->tm_mday;
  ss << " " << setfill('0') << setw(2) << ts->tm_hour;
  ss << ":" << setfill('0') << setw(2) << ts->tm_min;
  ss << ":" << setfill('0') << setw(2) << ts->tm_sec;

  return ss.str();
}

void parseSensorTypeBmp180Data(const uint8_t * buf, uint8_t bufSize, SQLiteDb & sqliteDb)
{
  syslog( LOG_NOTICE, "Receiced %d bytes from \'BMP 180\' air pressure and temperature sensor.\n", bufSize);
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
  const float temperatureInCelsius = temperature / 1000.;
  syslog( LOG_NOTICE, "%s : received temperature = %.2f °C.\n", GetCurrentTime().c_str(), temperatureInCelsius );

  stringstream ss;
  ss << "insert into wheathers (timestamp, temperature, pressure) values (\"" << GetCurrentTime()<< "\", " 
     << temperatureInCelsius 
     << ", 995.13);";
  
  sqliteDb.Execute(ss.str().c_str());
}

//we want to run as service
void daemonize() {
  /* Our process ID and Session ID */
    pid_t pid, sid;
    
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
            exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);
                
    /* Open any logs here */        
                
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
        
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log the failure */
        exit(EXIT_FAILURE);
    }
        
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
        
    /* Daemon-specific initialization goes here */

}

int main(int argc, char** argv){
 
  //daemonize();//we want to run as service
  openlog ( "homesensord", LOG_PID | LOG_CONS| LOG_NDELAY, LOG_USER);
  syslog( LOG_NOTICE, "Start sensor data receiver daemon.");

  SQLiteDb sqliteDb;
  if ( sqliteDb.Open(SQLITE_FILE_NAME) ) 
  {
    syslog( LOG_NOTICE, "Error opening database file \"%s\".", SQLITE_FILE_NAME);
    return -1;
  }


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
		  parseSensorTypeBmp180Data(&buf[headerSize], sensorDataSize, sqliteDb);
		  break;
		default:
		  printf("Unknown sensor type.\n");
		  break;
	}
	//delay(100);//nicht besser
	delay(925); //Delay after payload responded to, minimize RPi CPU time
    }
  } // forever loop
  return 0;
}



