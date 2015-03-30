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

#ifndef BIT_DECODER_H
#define BIT_DECODER_H 1

#include "stream_decoder.h"
#include <stdint.h>

/* interface for bit decoder */
typedef struct {
  char *name;
  char *shorthand;
  // interface
  int (*init)(stream_decoder_t *next);
  int (*input)(int transmission[], unsigned int length, int noise, int signal);
} bit_decoder_t;



#endif
