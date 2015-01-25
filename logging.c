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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int verbose = 0;
int l_n = 0;

#define LOGGING_MODULES 16
#define LOGGING_LENGTH 256
char l_status[LOGGING_MODULES][LOGGING_LENGTH];

void logging_init(void) {
  int i;
  for (i = 0; i<LOGGING_MODULES; i++)
    l_status[i][0] = 0;
}
void logging_destatus(void){
  if (l_n == 0) return;
  fprintf( stderr, "\r" );
  while (l_n > 0) { fprintf( stderr, " " ); l_n--; }
  fprintf( stderr, "\r" );
}
void logging_restatus(void){
  int i;
  logging_destatus();
  for (i = 0; i<LOGGING_MODULES; i++) {
    l_n += fprintf( stderr, "%s", l_status[i] );
  }
}
void _logging_status(int module,  const char* f, ... ) {
  if ((module >= LOGGING_MODULES) || (module < 0)) return;
  va_list argp;
  va_start(argp, f);
  if (vsprintf( l_status[module], f, argp ) < 0)
    l_status[module][0] = 0;
  va_end(argp);
}

void _logging_verbose( const char* f, ... ) {
  if (verbose > 3) {
    logging_destatus();
    va_list argp;
    va_start(argp, f);
    vfprintf( stderr, f, argp );
    va_end(argp);
  }
}
void _logging_info( const char* f, ... ) {
  if (verbose > 2) {
    logging_destatus();
    va_list argp;
    va_start(argp, f);
    vfprintf( stderr, f, argp );
    va_end(argp);
  }
}
void _logging_warning( const char* f, ... ) {
  if (verbose > 1) {
    logging_destatus();
    va_list argp;
    va_start(argp, f);
    vfprintf( stderr, f, argp );
    va_end(argp);
  }
}
void _logging_error( const char* f, ... ) {
  if (verbose > 0) {
    logging_destatus();
    va_list argp;
    va_start(argp, f);
    vfprintf( stderr, f, argp );
    va_end(argp);
  }
}
void _logging_message( const char* f, ... ) {
  if (verbose > -1) {
    logging_destatus();
    va_list argp;
    va_start(argp, f);
    vfprintf( stderr, f, argp );
    va_end(argp);
  }
}
