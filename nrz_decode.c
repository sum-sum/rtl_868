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

/** Non-Return-to-Zero decoding routine
 */

#include "bit_decoder.h"
#include "stream_decoder.h"
#include "nrz_decode.h"
#include "logging.h"

/// the minimum number of samples a new level must be
/// present before it is considered stable
#define LEVEL_THRESHOLD 2

/// maximum number of edges to be able to receive
/// set to number of bits of max. length transmission
/// any subsequent edge will be silently ignored
#define EDGE_TIMES_LEN 256
/// maximum bittime in histogram to consider for bitlen determination
#define HIST_LEN 64
/// averaging of bittimes in histogram
#define HIST_AVG 2
#define DATA_LEN (EDGE_TIMES_LEN/8)

stream_decoder_t *nrz_next;
unsigned int nrz_ok, nrz_err;

int nrz_init(stream_decoder_t *next) {
  if (next == 0) return -1;
  nrz_next = next;
  nrz_ok = 0;
  nrz_err = 0;
  logging_info( "NRZ Decoder initialized.\n" );
}

int nrz_input(int transmission[], unsigned int length, int noise, int signal) {
  /* decode the bits in transmission (1 per index) using NRZ */
  logging_verbose( "Got new transmission of length %i.\n", length );
  //
  // 0) convert bits to debounced edge times
  int level_counter = 0;
  int edge_time = 0;
  unsigned int edge_times[EDGE_TIMES_LEN];
  unsigned int edge_times_i = 0;
  int last_level = 0; // always start with level zero
  int i;
  
  for (i = 0; i<length; i++) {
    edge_time++;
    // decrease or increase the counts for this level
    if (transmission[i] > (signal + noise) >> 1) {
      level_counter++;
    } else if (transmission[i] < -(signal + noise) >> 1) {
      level_counter--;
    }
    if (level_counter < 0) level_counter = 0;
    if (level_counter > LEVEL_THRESHOLD) level_counter = LEVEL_THRESHOLD;
    if (((last_level == 0) && (level_counter == LEVEL_THRESHOLD)) || ((last_level == 1) && (level_counter == 0))) {
      // level has changed
      last_level = 1 - last_level;
      if (edge_times_i >= EDGE_TIMES_LEN) edge_times_i = EDGE_TIMES_LEN - 1;
      edge_times[edge_times_i++] = edge_time;
      edge_time = 0;
    }
  }
  // the end of transmission is also an edge
  if (edge_times_i >= EDGE_TIMES_LEN) edge_times_i = EDGE_TIMES_LEN - 1;
  edge_times[edge_times_i++] = edge_time;
  logging_verbose( "Tranmission contains %i edges.\n", edge_times_i );
  /// 1) convert the edge times to histogram
  unsigned int hist[HIST_LEN];
  for (i = 0; i<HIST_LEN; i++) hist[i] = 0;
  /* skip the first edge, start with 1 */
  logging_verbose( "Edges are at times: " );
  for (i = 1; i<edge_times_i; i++) {
    _logging_verbose( "%i, ", edge_times[i] );
    if ((edge_times[i] > 0) && (edge_times[i] < HIST_LEN))
      hist[edge_times[i]]++;
  }
  _logging_verbose( "\n" );
  /// find the maximum of the histogram as bittime
  unsigned int hist_max = 0;
  unsigned int hist_max_i = 0;
  logging_verbose( "Histogram is: " );
  for (i = 1; i<HIST_LEN; i++) {
    _logging_verbose( "%i ", hist[i] );
    if (hist[i] > hist_max) {
      hist_max = hist[i];
      hist_max_i = i;
    }
  }
  _logging_verbose( "\n" );
  if (hist_max_i == 0) {
    logging_warning( "Found no histogram max index.\n" );
    return -2;
  } else {
    logging_verbose( "Histogram max is at %i.\n", hist_max_i );
  }
  /// and calculate the bittime from multiple histogram entries
  unsigned int multbitlen;
  unsigned int multbitnum;
  multbitlen = hist[hist_max_i] * hist_max_i;
  multbitnum = hist[hist_max_i];
  for (i = 1; i < HIST_AVG; i++){
    if ((hist_max_i + i < HIST_LEN) && (hist_max_i - i > 0)) {
      multbitlen += 
        hist[hist_max_i-i] * (hist_max_i - i) +
        hist[hist_max_i+i] * (hist_max_i + i);
      multbitnum +=
        hist[hist_max_i-i] +
        hist[hist_max_i+i];
    }
  }
  float bitlen = 1.0 * multbitlen / multbitnum;
  logging_info( "Tranmission bit length is %1.2f.\n", bitlen );
  /// 2) convert the edges to bits using bitlen
  /* now decode the data */
  int data[DATA_LEN];
  data[0] = 0;
  unsigned int datai = 0;
  unsigned int datab = 0;
  int level = 0;
  float t = 0;
  for (i = 1 + (transmission[1] == 1 ? 1 : 0); i < edge_times_i; i++ ) {
    level = 1 - level;
    if (edge_times[i] > 32*bitlen) {
      continue;
    }
    t = edge_times[i];
    for (;t > bitlen / 2; t -= bitlen) {
      data[datai] <<= 1;
      data[datai] |= level;
      datab++;
      if (datab >= 8) {
        datab = 0;
        datai++;
        if (datai >= DATA_LEN) {
          logging_error( "No more memory.\n" );
          datai = DATA_LEN - 1;
        }
        data[datai] = 0;
      }
    }
    if (abs(t) > bitlen / 5)
      logging_info( "Remainder of time is large at bit %i: %1.3f samples at %1.3f bitlen.\n", datai * 8 + datab, t, bitlen );
  }
  // shift the last byte so that the first bit starts at MSB
  if (datab != 0) {
    for (i = datab;(i & 7) != 0; i++) {
      data[datai] <<= 1;
    }
    datai++;
  }
  logging_info( "Transmission has %i bits packed in %i bytes: ", datab + (datai-1)*8, datai );
  for (i = 0; i<datai; i++)
    _logging_info( "%02x ", data[i] );
  _logging_info( "\n" );
  /// 3) handle to next decoder
  if (nrz_next->input( data, datai ) == 0)
    nrz_ok++;
  else
    nrz_err++;
  // update status
  logging_status( 2, "bl=%1.2fS/b tl=%ib nerr=%i nok=%i", bitlen, datab + (datai-1)*8, nrz_err, nrz_ok );
  return 0;
}

bit_decoder_t nrz = {
  .name = "Non-Return-to-Zero decoder for bipolar signals",
  .shorthand = "nrz",
  .init = nrz_init,
  .input = nrz_input
};


