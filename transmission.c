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


#include <stdint.h>
#include "sample_decoder.h"
#include "transmission.h"
#include "logging.h"

/// maximum number of samples within one transmission,
/// everything exceeding this will be silently dropped
#define TD_SAMPLES_LEN  1024


typedef int16_t td_sample_t;
typedef int32_t td_sample2x_t;

/* where to handle received samples to */
bit_decoder_t *td_next;
int td_samples[TD_SAMPLES_LEN];
unsigned int td_samples_i;

int td_init( bit_decoder_t *next ) {
  if (next == 0) return -1;
  td_next = next;
  td_samples_i = 0;
  logging_info( "Transmission decoder initialized.\n" );
}


td_sample2x_t td_mean = 500<<(sizeof(td_sample_t)*8);
int td_transtime;
td_sample2x_t td_sigpwr;

/// threshold: this many samples required into either
/// direction to detect a transmission
#define TRANSMISSION_THRESHOLD 10
/// this much must the amplitude exceed the mean amplitude
/// for digital value detection
#define SAMPLE_AMPLITUDE_FACTOR 2

int td_input( td_sample_t sample ) {
  // memorize the amplitude
  td_sample_t sample_amplitude = td_mean >> (sizeof(td_sample_t)*8);
  td_mean += abs(sample) - (sample_amplitude);
  // check for transmission
  int new_transtime = td_transtime;
  if ((sample > SAMPLE_AMPLITUDE_FACTOR * sample_amplitude) || (sample < - SAMPLE_AMPLITUDE_FACTOR * sample_amplitude)) {
    new_transtime++;
  } else {
    new_transtime--;
  }
  if (new_transtime > 2*TRANSMISSION_THRESHOLD) {
    new_transtime = 2*TRANSMISSION_THRESHOLD;
  } else if (new_transtime < 0) {
    new_transtime = 0;
  }
  // determine the new level of the incoming sample
  int new_level;
  if ((sample > 0)) new_level = 1;
  else new_level = 0;
  /* now check if start of transmission */
  if ((new_transtime > TRANSMISSION_THRESHOLD) && (td_transtime <= TRANSMISSION_THRESHOLD)) {
    logging_verbose( "Transmission started with level %i.\n", new_level );
    td_samples_i = 0;
  }
  /* input bit if during transmission */
  if ((new_transtime > TRANSMISSION_THRESHOLD)) {
    if (td_samples_i >= TD_SAMPLES_LEN) td_samples_i = TD_SAMPLES_LEN - 1;
    td_samples[td_samples_i++] = new_level;
    // memorize the signal amplitude
    td_sigpwr += abs(sample);
  }
  /* check for end of transmission */
  if ((new_transtime <= TRANSMISSION_THRESHOLD) && (td_transtime > TRANSMISSION_THRESHOLD)) {
    if (td_samples_i < 3 * TRANSMISSION_THRESHOLD) {
      logging_verbose( "Dropping transmission, too shoft: %i samples, noise floor=%i, signal=%1.0f.\n", td_samples_i, (td_mean>>(sizeof(td_sample_t)*8)), (float)td_sigpwr/(float)td_samples_i );
    } else {
      logging_info( "Got Transmission of %i samples, noise floor=%i, signal=%1.0f.\n", td_samples_i, (td_mean>>(sizeof(td_sample_t)*8)), (float)td_sigpwr/(float)td_samples_i );
      logging_status( 1, "n=%i, s=%1.0f, l=%i", (td_mean>>(sizeof(td_sample_t)*8)), (float)td_sigpwr/(float)td_samples_i, td_samples_i );
      td_next->input( td_samples, td_samples_i );
    }
    td_sigpwr = 0;
  }
  td_transtime = new_transtime;
}

sample_decoder_t td = {
  .name = "Transmission decoder for bipolar signals (e.g. FM).",
  .shorthand = "td",
  .init = td_init,
  .input = td_input
};
