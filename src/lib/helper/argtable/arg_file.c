/*********************************************************************
This file is part of the argtable2 library.
Copyright (C) 1998-2001,2003-2011 Stewart Heitmann
sheitmann@users.sourceforge.net

The argtable2 library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
**********************************************************************/

/* config.h must be included before anything else */
#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#else
#error "can't find the C standard library"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
#error "can't find the C string library"
#endif

#include "argtable2.h"

#ifdef WIN32
# define FILESEPARATOR1 '\\'
# define FILESEPARATOR2 '/'
#else
# define FILESEPARATOR1 '/'
# define FILESEPARATOR2 '/'
#endif

/* local error codes */
enum {EMINCOUNT = 1, EMAXCOUNT};


static void resetfn (struct arg_file* parent) {
  /*printf("%s:resetfn(%p)\n",__FILE__,parent);*/
  parent->count = 0;
  }


/* Returns ptr to the base filename within *filename */
static const char* arg_basename (const char* filename) {
  const char* result = NULL, *result1, *result2;

  /* Find the last occurrence of eother file separator character. */
  /* Two alternative file separator chars are supported as legal  */
  /* file separators but not both together in the same filename.  */
  result1 = (filename ? strrchr (filename, FILESEPARATOR1) : NULL);
  result2 = (filename ? strrchr (filename, FILESEPARATOR2) : NULL);

  if (result2) {
    result = result2 + 1;  /* using FILESEPARATOR2 (the alternative file separator) */
    }

  if (result1) {
    result = result1 + 1;  /* using FILESEPARATOR1 (the preferred file separator) */
    }

  if (!result) {
    result = filename;  /* neither file separator was found so basename is the whole filename */
    }

  /* special cases of "." and ".." are not considered basenames */
  if (result && (strcmp (".", result) == 0 || strcmp ("..", result) == 0)) {
    result = filename + strlen (filename);
    }

  return result;
  }


/* Returns ptr to the file extension within *basename */
static const char* arg_extension (const char* basename) {
  /* find the last occurrence of '.' in basename */
  const char* result = (basename ? strrchr (basename, '.') : NULL);

  /* if no '.' was found then return pointer to end of basename */
  if (basename && !result) {
    result = basename + strlen (basename);
    }

  /* special case: basenames with a single leading dot (eg ".foo") are not considered as true extensions */
  if (basename && result == basename) {
    result = basename + strlen (basename);
    }

  /* special case: empty extensions (eg "foo.","foo..") are not considered as true extensions */
  if (basename && result && result[1] == '\0') {
    result = basename + strlen (basename);
    }

  return result;
  }


static int scanfn (struct arg_file* parent, const char* argval) {
  int errorcode = 0;

  if (parent->count == parent->hdr.maxcount) {
    /* maximum number of arguments exceeded */
    errorcode = EMAXCOUNT;
    }

  else if (!argval) {
    /* a valid argument with no argument value was given. */
    /* This happens when an optional argument value was invoked. */
    /* leave parent arguiment value unaltered but still count the argument. */
    parent->count++;
    }

  else {
    parent->filename[parent->count]  = argval;
    parent->basename[parent->count]  = arg_basename (argval);
    parent->extension[parent->count] = arg_extension (parent->basename[parent->count]); /* only seek extensions within the basename (not the file path)*/
    parent->count++;
    }

  /*printf("%s:scanfn(%p) returns %d\n",__FILE__,parent,errorcode);*/
  return errorcode;
  }


static int checkfn (struct arg_file* parent) {
  int errorcode = (parent->count < parent->hdr.mincount) ? EMINCOUNT : 0;
  /*printf("%s:checkfn(%p) returns %d\n",__FILE__,parent,errorcode);*/
  return errorcode;
  }


static void errorfn (struct arg_file* parent, FILE* fp, int errorcode, const char* argval, const char* progname) {
  const char* shortopts = parent->hdr.shortopts;
  const char* longopts  = parent->hdr.longopts;
  const char* datatype  = parent->hdr.datatype;

  /* make argval NULL safe */
  argval = argval ? argval : "";

  fprintf (fp, "%s: ", progname);

  switch (errorcode) {
    case EMINCOUNT:
      fputs ("missing option ", fp);
      arg_print_option (fp, shortopts, longopts, datatype, "\n");
      break;

    case EMAXCOUNT:
      fputs ("excess option ", fp);
      arg_print_option (fp, shortopts, longopts, argval, "\n");
      break;

    default:
      fprintf (fp, "unknown error at \"%s\"\n", argval);
    }
  }


struct arg_file* arg_file0 (const char* shortopts,
                            const char* longopts,
                            const char* datatype,
                            const char* glossary) {
  return arg_filen (shortopts, longopts, datatype, 0, 1, glossary);
  }


struct arg_file* arg_file1 (const char* shortopts,
                            const char* longopts,
                            const char* datatype,
                            const char* glossary) {
  return arg_filen (shortopts, longopts, datatype, 1, 1, glossary);
  }


struct arg_file* arg_filen (const char* shortopts,
                            const char* longopts,
                            const char* datatype,
                            int mincount,
                            int maxcount,
                            const char* glossary) {
  size_t nbytes;
  struct arg_file* result;

  /* foolproof things by ensuring maxcount is not less than mincount */
  maxcount = (maxcount < mincount) ? mincount : maxcount;

  nbytes = sizeof (struct arg_file)    /* storage for struct arg_file */
           + sizeof (char*)* maxcount /* storage for filename[maxcount] array */
           + sizeof (char*)* maxcount /* storage for basename[maxcount] array */
           + sizeof (char*)* maxcount; /* storage for extension[maxcount] array */

  result = (struct arg_file*) malloc (nbytes);

  if (result) {
    int i;

    /* init the arg_hdr struct */
    result->hdr.flag      = ARG_HASVALUE;
    result->hdr.shortopts = shortopts;
    result->hdr.longopts  = longopts;
    result->hdr.glossary  = glossary;
    result->hdr.datatype  = datatype ? datatype : "<file>";
    result->hdr.mincount  = mincount;
    result->hdr.maxcount  = maxcount;
    result->hdr.parent    = result;
    result->hdr.resetfn   = (arg_resetfn*) resetfn;
    result->hdr.scanfn    = (arg_scanfn*) scanfn;
    result->hdr.checkfn   = (arg_checkfn*) checkfn;
    result->hdr.errorfn   = (arg_errorfn*) errorfn;

    /* store the filename,basename,extension arrays immediately after the arg_file struct */
    result->filename  = (const char**) (result + 1);
    result->basename  = result->filename + maxcount;
    result->extension = result->basename + maxcount;
    result->count = 0;

    /* foolproof the string pointers by initialising them with empty strings */
    for (i = 0; i < maxcount; i++) {
      result->filename[i] = "";
      result->basename[i] = "";
      result->extension[i] = "";
      }
    }

  /*printf("arg_filen() returns %p\n",result);*/
  return result;
  }
