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
project ( lib-argtable )

# Set-up CMake
cmake_minimum_required ( VERSION 2.6 )

# Enforce the following policies
#if(COMMAND cmake_policy)
#  cmake_policy(SET CMP0003 NEW)
#  cmake_policy(SET CMP0015 NEW)
#endif(COMMAND cmake_policy)

# Include the checks for library functions
include ( CheckLibraryExists )

# Include the checks for header files
include ( CheckIncludeFiles )

##
## Project Configuration
##

# Set the global configure file
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/config/config.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/config/config.h )

# Add the global configure file to the search path
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/config )

##
## Library Sources
##

# Add the YAML library headers to the search path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# On Windows we need to include the getopt routines, as well as the library
# source
IF( WIN32 )
  ADD_LIBRARY( argtable STATIC
    arg_dbl.c
    arg_end.c 
    arg_file.c 
    arg_int.c 
    arg_lit.c
    arg_rem.c 
    arg_str.c         
    argtable2.c 
    getopt.c 
    getopt1.c )
ELSE()
  ADD_LIBRARY( argtable STATIC 
    arg_date.c 
    arg_dbl.c 
    arg_end.c 
    arg_file.c
    arg_int.c 
    arg_lit.c 
    arg_rem.c        
    arg_rex.c 
    arg_str.c
    argtable2.c
    getopt.c 
    getopt1.c )
ENDIF( )

#SET_TARGET_PROPERTIES( argtable2 PROPERTIES DEBUG_POSTFIX d )
