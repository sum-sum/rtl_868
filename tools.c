
/** From Chromium Repository, crc8.c, adapted to allow varying polynoms by Clemens Helfmeier */
// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdint.h>
#include "logging.h"

uint8_t crc8(uint16_t poly, uint8_t *vptr, int len)
{
  const uint8_t *data = vptr;
  uint16_t crc = 0;
  int i, j;
  for (j = len; j; j--, data++) {
    crc ^= (*data << 8);
    for(i = 8; i; i--) {
      if (crc & 0x8000)
        crc ^= (poly << 7);
      crc <<= 1;
    }
  }
  return (uint8_t)(crc >> 8);
}

int search_magic(int transmission[], unsigned length, uint8_t tm[], int tm_length, int magic[], int magic_length) {
  /* search for magic_lenght bits (given by magic[]) within transmission[] and return the
   * properly shifted version of transmission[] in tm[], so that
   * tm[0] is magic[0], etc.
   * tm_length must be larger than magic_length!
   * return number of bytes within tm.
   */
  unsigned int shift;
  unsigned int ofs;
  unsigned int shf;
  int inv = 0;
  unsigned int i = 0;
  for (shift = 0; shift < length*8; shift++) {
    ofs = shift >> 3;
    shf = shift & 7;
    // check if we would already exceed packet size with preamble and length field
    if (magic_length + (shift + 7) >> 3 > length) {
      logging_warning( "Transmission too short: %i.\n", length );
      return 0;
    }
    for (i = 0; i <= (magic_length | 0x07) >> 3; i++) {
      tm[i] = ((transmission[i+ofs+1] | (transmission[i+ofs]<<8)) >> (8-shf)) & 0xFF;
    }
    // now check tm
    int found = 0;
    int msk;
    for (i = 0; i < (magic_length | 0x07) >> 3; i++) {
      if (i << 3 > magic_length) {
        msk = (1<<(magic_length & 0x07)) - 1;
      } else {
        msk = 0xFF;
      }
      if ((tm[i] & msk) != (magic[i] & msk))
        found |= 1;
      if (((tm[i] & msk) ^ 0xFF) != (magic[i] & msk))
        found |= 2;
      if (found == 0x03) {
        break;
      }
    }
    // bit in found is set if at least one of the
    // magic bytes did not match
    if (found == 0x01) {
      inv = 0xFF;
      break;
    }
    if (found == 0x02) {
      inv = 0x00;
      break;
    }
  }
  if (shift >= length*8) {
    logging_warning( "No preamble detected. Ignoring dataset.\n" );
    return 0;
  }
  // fill the shifted data into tm and empty bits with 0
  for (i = 0; (i<tm_length/sizeof(tm[0])); i++) {
    if (i+ofs+1 < length) {
      tm[i] = ((transmission[i+ofs+1] | (transmission[i+ofs]<<8)) >> (8-shf)) & 0xFF;
    } else if (i+ofs < length) {
      tm[i] = (((transmission[i+ofs]<<8)) >> (8-shf)) & 0xFF;
    } else {
      tm[i] = 0;
    }
    tm[i] ^= inv;
  }
  ofs = length - ofs; // number of filled bytes in tm
  if (ofs >= tm_length/sizeof(tm[0])) {
    ofs = tm_length/sizeof(tm[0]);
  }
  return ofs;
}
