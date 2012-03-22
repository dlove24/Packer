#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __GNUC__
#include <unistd.h>
#else
#include <io.h>
#endif
#include "bstrlib.h"

#ifndef CLOCKS_PER_SEC
# ifdef CLK_TCK
#  define CLOCKS_PER_SEC (CLK_TCK)
# endif
#endif

/*
 *  reverseTest is a function written by Doug Bagley and is part of the "Great
 *  Computer Language Shootout": http://www.bagley.org/~doug/shootout/
 *
 *  This function is I/O limited and therefore there is very little hope
 *  for significant performance differences to be measured.
 *
 *  It should be noted however, that the code for reading the data is
 *  quite convoluted, and the code which follows it takes a little bit
 *  of analysis to see how it works and that it is correct.
 */
#define MAXREAD 4096

static int reverseTest (void) {
  int nread, len = 0, size = (4 * MAXREAD);
  char* cp, *offset, *buf = malloc (size + 1);

  for (;;) {
    if ( (nread = read (0, (buf + len), MAXREAD)) > 0) {
      len += nread;

      if (MAXREAD > (size - len)) {
        size <<= 1;

        if (0 == (buf = realloc (buf, size + 1))) {
          fprintf (stderr, "realloc failed\n");
          exit (1);
          }
        }
      }

    else {
      if (0 == nread) {
        break;
        }

      if (-1 == nread) {
        perror ("read");
        exit (1);
        }
      }
    }

  offset = buf + len;
  *offset = (char) '\0';

  for (cp = offset; cp > buf; --cp) {
    if ('\n' == *cp) {
      *offset = (char) '\0';

      if (cp < offset) {
        fputs (offset = cp + 1, stdout);
        }
      }
    }

  if (cp < offset) {
    *offset = (char) '\0';
    fputs (cp, stdout);
    }

  free (buf);
  return 0;
  }

/*
 *  bstrReverseTest is a simple rerendering of the above using the bstrlib
 *  library.  In terms of simplicity I think the following code compares
 *  favorably with the code written for Java, or straight C and C++.
 *
 *  This example demonstrates how well bstrlib interoperates with the standard
 *  C library functions (fread, fputs) and the C languages' semantics for
 *  char buffer based strings.
 *
 *  Note that in terms of "correctness" this code is leveraging the fact that
 *  bstrings always allocate space for at least one additional character past
 *  the string length which is filled by bstrlib functions with a '\0'.  Thus
 *  even if the last character of the input is read as '\n' here, writing a
 *  '\0' one character past the end is well defined.
 */
static int bstrReverseTest (void) {
  bstring b = bread ( (bNread) fread, stdin);
  int i;

  for (i = blength (b) - 1; i >= 0; i--) {
    if (b->data[i] == '\n') {
      fputs ( (char*) b->data + i + 1, stdout);
      b->data[i + 1] = (char) '\0';
      }
    }

  fputs ( (char*) b->data, stdout);
  bdestroy (b);
  return 0;
  }

int main (int argc, char* argv[]) {
  int ret;
  clock_t c0, c1;

  argv = argv;
  c0 = clock ();

  if (argc >= 2) {
    ret = bstrReverseTest ();
    }

  else {
    ret = reverseTest ();
    }

  c1 = clock ();
  fprintf (stderr, "Time %.2fs\n", (c1 - c0) * (1.0 / (double) CLOCKS_PER_SEC));

  return ret;
  }

