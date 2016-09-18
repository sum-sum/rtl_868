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

// This type of data is received for example for ALDI type weather stations
// the devices at hand have a bug, after 23.7°C, the next (upwards) step is 23.0°C

#include "stream_decoder.h"
#include "logging.h"
#include "data_logger.h"
#include "tools.h"

data_logger_t *ws300_next;


int ws300_init( data_logger_t *next ) {
  if (next == 0) return -1;
  ws300_next = next;  
  logging_info( "WS300 decoder initialized.\n" );
}

int ws300_input(int transmission[], unsigned length) {
  /* decoding the result:
   *  aa aa 2d d4 51 11 4d 07 29 21 00
   *                             ^^ checksumme
   *                          ^^ rel. hum 0x29 = 41%
   *                    ^^ ^^ Temperatur in Vorkomma & Nachkomma in °C bezogen auf -50°C
   *                          0x4d07 = -50°C + 77°C + 0.7°C = 27.7°C
   *                  ^ Hauscode: 1...15 = 1...f
   *                 ^ Kanal: 1...3 = 1...3, 4 = 0
   *  ^^^^^ preamble
   * 
   */
  int ofs;
  int i;
  int magic[] = {0xaa, 0x2d, 0xd4};
  uint8_t tm[11];
  // find magic
  ofs = search_magic( transmission, length, tm, sizeof(tm)/sizeof(tm[0]), magic, 8*sizeof(magic)/sizeof(magic[0]) );
  if (ofs < 8) {
    logging_warning( "Transmission too short: %i.\n", ofs );
    return -2;
  }
  // check the preamble
  if (!(tm[3] == 0x51)) {
    logging_warning( "Invalid preamble field %02x detected. Ignoring dataset.\n", tm[3] );
    return -3;
  }
  // check the checksum
  int crc = (tm[3] + tm[4] + tm[5] + tm[6] + tm[7] + tm[8]) & 0xFF;
  if (crc != 0) {
    logging_warning( "Invalid checksum %02x detected. Ignoring dataset.\n", crc );
    return -4;
  }
  // the transmissionset is correct
  int hauscode = tm[4] & 0x0F;
  int channel = ((tm[4]>>4) & 0x3);
  if (channel == 0) channel = 4;
  float rel_hum = 1.0 * tm[7];
  float temp = 1.0 * tm[5] + 0.1 * tm[6] - 50.0;
  logging_info( "Recieved dataset: hauscode=%i, channel=%i, temp=%1.1f°C, rel_hum=%1.0f%%.\n", hauscode, channel, temp, rel_hum );
  return ws300_next->input( (hauscode<<8) | channel, temp, rel_hum, 0 );
}
  
  
stream_decoder_t ws300 = {
  .name = "Decoder for WS-300 weather stations.",
  .shorthand = "ws300",
  .init = ws300_init,
  .input = ws300_input
};


  
