#
#    rtl_868
#    Copyright (C) 2015  Clemens Helfmeier
#    e-mail: clemenshelfmeier@gmx.de
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# for debugging
# CFLAGS += -ggdb
#-l : 
LDFLAGS += -lrt
W32_LDFLAGS += 

rtl_868: ws300.o transmission.o nrz_decode.o main.o logging.o tx29.o tools.o data_logger.o
	${CC} ${LDFLAGS} $^ -o $@

rtl_868.exe: ws300.o transmission.o nrz_decode.o main.o logging.o tx29.o tools.o data_logger.o
	${CC} ${W32_LDFLAGS} $^ -o $@

%.lss: %
	objdump -xS $< > $@
