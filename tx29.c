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

// this is some other code, very similar to what is known as TX29
// on the internet.

#include "stream_decoder.h"
#include "logging.h"
#include "tools.h"
#include "data_logger.h"

data_logger_t *tx29_next;

int tx29_init( data_logger_t *next ) {
  if (next == 0) return -1;
  tx29_next = next;
  logging_info( "TX29 decoder initialized.\n" );
}

int tx29_input(int transmission[], unsigned length) {
  // find preamble it is 2d d4 and stuff before must be aa
  unsigned int ofs;
  int i;
  int magic[] = {0xaa, 0x2d, 0xd4};
  uint8_t tm[11];
  // find magic
  ofs = search_magic( transmission, length, tm, sizeof(tm)/sizeof(tm[0]), magic, 8*sizeof(magic)/sizeof(magic[0]) );
  if (ofs == 0) {
    return;
  }
  //
  logging_info( "Data packet after preamble detection: %i -> ", ofs );
  for (i = 0; (i<sizeof(tm)/sizeof(tm[0])); i++) {
    _logging_info( "%02x ", tm[i] );
  }
  _logging_info( ".\n" );
  // decode length
  unsigned len = (tm[3]>>4) & 0x0F;
  len = (len + 1) >> 1; // convert to bytes
  if (len > ofs - 4) {
    logging_warning( "Invalid length field %i. Ignoring dataset.\n", len );
    return -4;
  }
  // check the checksum
  uint8_t crc = crc8(0x131, &tm[3], len);
  if (crc != 0) {
    logging_warning( "Invalid checksum %02x detected. Ignoring dataset.\n", crc );
    return -6;
  }
  int sensid;
  int newbatt;
  float temp;
  float rel_hum;
  int weakbatt;
  if (len == 5) {
    // the tm is correct
    sensid = ((tm[3] & 0x0f) << 2) | ((tm[4]>>6) & 0x03);
    newbatt = (tm[4] >> 5) & 1;
    temp = 10.0 * (tm[4] & 0x0F) +  1.0 *((tm[5]>>4) & 0x0F) + 0.1 * (tm[5] & 0x0F) - 40.0;
    weakbatt = (tm[6]>>7) & 1;
    rel_hum = 1.0 * (tm[6] & 0x7F);
  } else {
    logging_warning( "Don't know how to handle data of length %i.\n", len );
    return -5;
  }
  logging_info( "Recieved dataset: sensid=%i, newbatt=%i, weakbatt=%i, temp=%1.1f°C, rel_hum=%1.0f%%.\n", sensid, newbatt, weakbatt, temp, rel_hum );
  return tx29_next->input( sensid, temp, rel_hum, newbatt | (weakbatt << 1) );
}


stream_decoder_t tx29 = {
  .name = "Decoder for TX29 weather stations.",
  .shorthand = "tx29",
  .init = tx29_init,
  .input = tx29_input
};


