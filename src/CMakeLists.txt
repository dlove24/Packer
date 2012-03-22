# Copyright (c) 2011 David Love
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

##
## Project Definition
##

# Project Name
project ( braid )

# Set-up CMake
cmake_minimum_required ( VERSION 2.6 )

# Include the checks for library functions
include ( CheckLibraryExists )

# Include the checks for header files
include ( CheckIncludeFiles )

##
## Compiler Configuration
##

# Enforce ANSI mode on GCC compilers
if ( ${CMAKE_COMPILER_IS_GNUCC} ) 
  message ( "-- Using GCC compiler profile...")
  # Force ANSI C (C89/C90) mode
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -Wextra" )
endif ( ${CMAKE_COMPILER_IS_GNUCC} ) 

##
## Project Configuration
##

# Set the global configure file
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/lib/config/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/config/config.h )

# Add the global configure file to the search path
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/lib/config )

##
## Library Configuration
##

# Look for the math library: on some platforms this is part of the C
# library, on others it needs linking separately
check_library_exists ( m floor "" SEPARATE_LIB_M ) 

if ( SEPARATE_LIB_M )
  # If the math library is separate, then it needs to be included
  # in the global link flags
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lm")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -lm")
endif( SEPARATE_LIB_M )

# Look for the vsnprintf function, used by the bstring library
check_library_exists ( c vsnprintf "" BSTRLIB_VSNP_OK )

# Look for functions from the C Standard library. These should
# be present on most platforms...
check_library_exists ( c malloc "" HAVE_MALLOC )
check_library_exists ( c realloc "" HAVE_REALLOC )
check_library_exists ( c strdup "" HAVE_STRDUP )

# Look for various platform defines. This are usually present, but may
# be missing (especially on MS Windows)
check_library_exists ( c fileno "" HAVE_FILENO )
check_library_exists ( c localtime_r "" HAVE_LOCALTIME_R )
check_library_exists ( c pipe "" HAVE_PIPE )
check_library_exists ( c putenv "" HAVE_PUTENV )
check_library_exists ( c setenv "" HAVE_SETENV )
check_library_exists ( c sleep "" HAVE_SLEEP ) 
check_library_exists ( c strsignal "" HAVE_STRSIGNAL )
check_library_exists ( c unsetenv "" HAVE_UNSETENV )

##
## Include Files Defines for the C Standard Library
##

# Look for the "assert.h" header file
check_include_files ( assert.h HAVE_ASSERT_H )

# Look for the "inttypes.h" header file
check_include_files ( inttypes.h HAVE_INTTYPES_H )

# Look for the standard C library header file
check_include_files ( ctype.h HAVE_CTYPE_H )
check_include_files ( stdint.h HAVE_STDINT_H )
check_include_files ( stdio.h HAVE_STDIO_H )

# Look for the header files defining the constants for type lengths
check_include_files ( float.h HAVE_FLOAT_H )
check_include_files ( limits.h HAVE_LIMITS_H )
check_include_files ( values.h HAVE_VALUES_H )

# Look for the C standard library headers
check_include_files ( stdlib.h HAVE_STDLIB_H )

# Look for the C standard string library headers
check_include_files ( ctype.h HAVE_CTYPE_H )
check_include_files ( string.h HAVE_STRING_H )
check_include_files ( strings.h HAVE_STRINGS_H )

# Look for the C routines for formatted output conversions
check_include_files ( stdarg.h HAVE_STDARG_H )

##
## Include Files Defines for the POSIX Platform
##

# Look for various POSIX functions
check_include_files ( unistd.h HAVE_UNISTD_H 1 )

# Look for the POSIX thread library
check_include_files ( pthread.h HAVE_PTHREAD_H 1 )

##
## Include File Defines for Other Features
##

# Look for the getopt library headers
check_include_files ( getopt.h HAVE_GETOPT_H )

# Check whether we are on a 32-bit or a 64-bit platform for non-Apple
# platforms: Apple platforms have to be checked at compile time

if( NOT APPLE )
   # check 64 bit
   if(CMAKE_SIZEOF_VOID_P MATCHES 4)
     set( HAVE_64_BIT 0 )
   else()
     set( HAVE_64_BIT 1 )
   endif()

   # check endianness
   include( TestBigEndian )
   test_big_endian( HAVE_BIG_ENDIAN )
   if( HAVE_BIG_ENDIAN )
     set( HAVE_BIG_ENDIAN 1 )
   else()
     set( HAVE_BIG_ENDIAN 0 )
   endif()
endif()

##
## Compiler Options
##

# Enforce ANSI mode on GCC compilers
if ( ${CMAKE_COMPILER_IS_GNUCC} )
  message ( "-- Using GCC compiler profile...")

  # Force ANSI C (C89/C90) mode
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -Wextra" )
endif ( )

# Check for CLang (this should be redundant eventually, as a patch
# has been made to the 2.8 branch. However, until that version becomes
# common, this will do)
#
# NOTE: This MUST be done AFTER the GCC check, as Clang also shows up
# as GCC. So test for that first, then Clang. One day this will all go
# away..
if ( "${CMAKE_C_COMPILER_ID}" MATCHES "Clang" )
  message ( "-- Using CLang compiler profile...")
  # Force ANSI C (C89/C90) mode

  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall -Wextra" )
endif ( )

##
## Add Project Libraries
##

add_subdirectory( lib )

##
## Application Sources
##

add_subdirectory( bin )

##
## Test definitions.
##

# Enable CTest
#enable_testing()

# Build the test harnesses/frameworks/applications, but remove them from the 
# install list
#add_subdirectory(test 
#	EXCLUDE_FROM_ALL
#	)
