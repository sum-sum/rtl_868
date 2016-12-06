/*
    rtl_868
    Copyright (C) 2015  Clemens Helfmeier
    e-mail: clemenshelfmeier@gmx.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "logging.h"
#include "data_logger.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

/// time at start of program
time_t dl_file_start;

/// file to write to
FILE *dl_file_out;

int dl_file_init( FILE *out ) {
  time( &dl_file_start );
  dl_file_out = out;
  logging_info( "Data_Logger initialized.\n" );
}

int dl_file_input(int sensor_id, float temp, float rel_hum, int flags) {
  /* output into octave readable file */
  /* decorate with timestamp and seconds since start of program */
  
  time_t cur_time;
  struct tm ts;
  if ((time( &cur_time ) == 0) || (localtime_r(&cur_time, &ts) == 0)) {
    logging_error( "Could not get current time.\n" );
    ts.tm_year = 0; ts.tm_mon = 0; ts.tm_mday = 0;
    ts.tm_hour = 0; ts.tm_min = 0; ts.tm_sec = 0;
  } 
  double time_sec = difftime( cur_time, dl_file_start );

  logging_info( "%04i-%02i-%02i %02i:%02i:%02i, %lli, %i, %1.2f, %1.2f, %i.\n", ts.tm_year+1900, ts.tm_mon+1, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, (long long int)cur_time, sensor_id, temp, rel_hum, flags );
  if (rel_hum == 106) {
    fprintf( dl_file_out, "%04i-%02i-%02i %02i:%02i:%02i, %lli, %i, %1.2f, nan, %i.\n", ts.tm_year+1900, ts.tm_mon+1, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, (long long int)cur_time, sensor_id, temp, flags );
  } else {
    fprintf( dl_file_out, "%04i-%02i-%02i %02i:%02i:%02i, %lli, %i, %1.2f, %1.2f, %i.\n", ts.tm_year+1900, ts.tm_mon+1, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec, (long long int)cur_time, sensor_id, temp, rel_hum, flags );
  }
  fflush( dl_file_out );
  
  logging_status( 3, "%i -> %1.1f°C, %1.1f%%", sensor_id, temp, rel_hum );
  return 0; // ok :-)
}

data_logger_t dl_file = {
  .name = "DataLogger writing to file",
  .shorthand = "dl_file",
  .init = dl_file_init,
  .input = dl_file_input,
};
