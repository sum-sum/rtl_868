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


#ifndef LOGGING_H
#define LOGGING_H 1

extern int verbose;
#define LOGGING_STRR(arg) #arg
#define LOGGING_STR(arg) LOGGING_STRR( arg )

#define logging_verbose( args... ) do { _logging_verbose( "V: " __FILE__ ":" LOGGING_STR(__LINE__) ": " args ); } while (0)
#define logging_info( args... ) do { _logging_info( "I: " __FILE__ ":" LOGGING_STR(__LINE__) ": " args ); } while (0)
#define logging_warning( args... ) do { _logging_warning( "W: " __FILE__ ":" LOGGING_STR(__LINE__) ": " args ); } while (0)
#define logging_error( args... ) do { _logging_error( "E: " __FILE__ ":" LOGGING_STR(__LINE__) ": " args ); } while (0)
#define logging_message( args... ) do { _logging_mesage( args ); } while (0)
#define logging_status( n, str, args... ) do { _logging_status( n,  str "; ", args ); } while (0)

void logging_restatus(void);
void _logging_status(int module,  const char* f, ... );
void _logging_verbose( const char* f, ... );
void _logging_info( const char* f, ... );
void _logging_warning( const char* f, ... );
void _logging_error( const char* f, ... );
void _logging_message( const char* f, ... );
void logging_init(void);
          

#endif
