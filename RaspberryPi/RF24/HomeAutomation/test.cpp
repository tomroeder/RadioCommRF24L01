#include <time.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

int main(void)
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

  cout << "Timestamp = " << ss.str() << endl;

  return 0;
}