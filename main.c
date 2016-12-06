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


#include "nrz_decode.h"
#include "transmission.h"
#include "ws300.h"
#include "logging.h"
#include "tx29.h"
#include "data_logger.h"

#include <unistd.h>
#include <stdarg.h>

#include <time.h>
#include <sys/timeb.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
#include "os/windows.h"
#endif
int data_to_string( float data, float* base, char* cexp ) {
  // convert the value of data to base + exponent notation
  int exp = 0;
  while ((data > (1<<10)) && (exp < 4)) {
    data /= (1<<10);
    exp++;
  }
  while ((data < 1) && (exp > -4)) {
    data *= (1<<10);
    exp--;
  }
  char lexp[] = "pnum kMGT";
  *cexp = lexp[ exp + 4 ];
  *base = data;
  return 1;
}

FILE *in, *out;
int duplicate_stream_input( int transmission[], unsigned int length ) {
  if ((ws300.input( transmission, length) == 0) ||
    (tx29.input(transmission, length) == 0))
    return 0;
  else if (verbose > 1) {
    if (length < 6) return -1;
    fprintf( out, "__, %i, ", length );
    while (length-- > 0)
      fprintf( out, "%02x ", *transmission++ );
    fprintf( out, "\n" );
    fflush(out);
    return 0;
  } else {
    return 0;
  }
}

int main (int argc, char **argv) {

  char* filename = 0;
  char* outfilename = 0;
  int c;
  
  logging_init();
  
  opterr = 0;
  
  while ((c = getopt (argc, argv, "vqf:o:")) != -1)
    switch (c)
    {
      case 'v':
        verbose++;
        break;
      case 'q':
        verbose--;
        break;
      case 'f':
        if (filename != 0) {
          logging_info( "Overriding previous -f flag '%s' with '%s'.\n", filename, optarg );
        }
        filename = optarg;
        break;
      case 'o':
        if (outfilename != 0) {
          logging_info( "Overriding previous -o flag '%s' with '%s'.\n", outfilename, optarg );
        }
        outfilename = optarg;
        break;
      case '?':
        if ((optopt == 'f') || (optopt == 'o') || (optopt == 'd'))
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        fprintf( stderr,
          "rtl_868, Copyright (C) 2015 Clemens Helfmeier\n"
          "rtl_868 comes with ABSOLUTELY NO WARRANTY; for details see the\n"
          "LICENSE file distributed with the program.\n"
          "This is free software, and you are welcome to redistribute it\n"
          "under certain conditions; see the LICENSE file for details.\n"
          "\n"
          "Usage: \n" 
          " rtl_868 [PARAMETERS] [FILENAME]\n"
          "   [FILENAME]     read input data from there. Defaults to stdin.\n"
          "   [PARAMETERS]   Unix style parameters with possible values:\n"
          "      -v          be more verbose. accumulates when given multiple times.\n"
          "      -q          be less verbose.\n"
          "      -f file     open file instead of stdin.\n"
          "      -o file     open file instead of stdout.\n"
          "\n"
        );
        return 1;
      default:
        abort ();
    }
  
  if ((argc - 1 == optind) && (filename == 0)) {
    logging_verbose( "Using positional argument '%s' as input filename.\n", argv[optind] );
    // if there is a single remaining argument, use it as filename
    filename = argv[optind];
  } else {
    int index;
    for (index = optind; index < argc; index++) {
      logging_error( "Unkown non-option argument %s\n", argv[index] );
      return 1;
    }
  }

  if (filename == 0) {
    filename = "-";
  }
  if (outfilename == 0){
    outfilename = "-";
  }
  logging_verbose( "decode.c version 0, verbose %i, filename %s -> %s.\n", verbose, filename, outfilename );
  logging_info(
     "rtl_868, Copyright (C) 2015 Clemens Helfmeier\n"
     "rtl_868 comes with ABSOLUTELY NO WARRANTY; for details see the\n"
     "LICENSE file distributed with the program.\n"
     "This is free software, and you are welcome to redistribute it\n"
     "under certain conditions; see the LICENSE file for details.\n"
  );
  
  if ((filename == 0) || (strcmp( filename, "-" ) == 0))
    in = stdin;
  else
    in = fopen( filename, "rb" );
  if (in == 0) {
    if (filename == 0) {
      logging_error( "Could not open stdin as input file.\n" );
    } else {
      logging_error( "Could not open input file '%s'.\n", filename );
    }
    return 1;
  }
#if defined(__MINGW32__) || defined(__MINGW64__)
  setmode(fileno(in), O_BINARY);
#endif
  if ((outfilename == 0) || (strcmp( outfilename, "-" ) == 0))
    out = stdout;
  else
    out = fopen( outfilename, "a" );
  if (out == 0) {
    if (outfilename == 0) {
      logging_error( "Could not open stdout as output file.\n" );
    } else {
      logging_error( "Could not open output file '%s'.\n", outfilename );
    }
    return 1;
  }


  stream_decoder_t mysd = { .init = 0, .input = &duplicate_stream_input };
  
  // construct the signal chain
  /* transmission decoder */
  td.init( &nrz );
  /* nrz */
  nrz.init( &mysd );
  /* ws300 and tx29 */
  ws300.init( &dl_file );
  tx29.init( &dl_file );
  /* dl_file */
  dl_file.init( out );
  
  uint16_t d[1024];
  unsigned long long int ndata = 0;
  unsigned long long int last_ndata = 0;
  
  struct timespec last_status;
#ifdef __APPLE__
  last_status.tv_sec = time(NULL);
  last_status.tv_nsec = 0;
#elif defined(__MINGW32__) || defined(__MINGW64__)
  last_status.tv_sec = time(NULL);
  last_status.tv_nsec = 0;
#else
  clock_gettime( CLOCK_MONOTONIC, &last_status );
#endif
  
  // read from stdin S16LE data
  while (1) {
    /* read a chunk */
    int n = fread( d, sizeof(d[0]), sizeof(d)/sizeof(d[0]), in );
    if (feof(in)) {
      logging_error( "\nEOF reached at %i.\n", ndata );
      break;
    }
    if (n == 0) {
      logging_error( "\n0 data readed at %i.\n", ndata );
      continue;
    } else {
      ndata += n;
    }

    struct timespec now;
#ifdef __APPLE__
    now.tv_sec = time(NULL);
    now.tv_nsec = 0;
#elif defined(__MINGW32__) || defined(__MINGW64__)
    now.tv_sec = time(NULL);
    now.tv_nsec = 0;
#else
    clock_gettime( CLOCK_MONOTONIC, &now );
#endif
    // if time is over, redisplay status
    float dt = 1.0 * (now.tv_sec - last_status.tv_sec) + 1.0 * (now.tv_nsec - last_status.tv_nsec) / 1e9;
    if (dt >= 1) {
      // calculate new throughput
      float throughput = (ndata - last_ndata) / dt;
      last_ndata = ndata;
    
      /* update status display */
      char nd_e;
      float nd_b;
      data_to_string( (float)ndata, &nd_b, &nd_e );
      char tp_e;
      float tp_b;
      data_to_string( throughput, &tp_b, &tp_e );
	  	
		if (verbose > 1) {
			logging_status( 0, "%s -> %s, %1.1f%c, %1.1f%c", filename, outfilename, nd_b, nd_e, tp_b, tp_e );
			logging_restatus();
		}
	  
      last_status.tv_sec = now.tv_sec;
      last_status.tv_nsec = now.tv_nsec;
    }
    
    /* and put the chunk into the transmission decoder */
    int i;
    for (i = 0; i<n; i++) {
      td.input( d[i] );
    }
  }
  printf("Program terminated\n");
  fclose(in);
}
