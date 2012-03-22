#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstrlib.h"

/*
 *  strcatTest is a function written by Doug Bagley and is part of the "Great
 *  Computer Language Shootout": http://www.bagley.org/~doug/shootout/
 *
 *  Doug Bagley did well to use a memory doubling policy (like bstrlib uses)
 *  to minimize the impact of memory management on the performance.  He also
 *  is smart to use the incrementally calculated strend pointer forward to
 *  minimize the performance drain of having to scan the whole string each
 *  time looking for the '\0' before copying "STUFF" to the end.
 *
 *  Nevertheless, these convolutions, that make the code harder to read and
 *  maintain, are insufficient to keep pace with the very simple to read and
 *  understand bstrlib version below.
 */
#define STUFF "hello\n"

#ifndef CLOCKS_PER_SEC
# ifdef CLK_TCK
#  define CLOCKS_PER_SEC (CLK_TCK)
# endif
#endif

static int strcatTest (int n) {
  int i, buflen = 32;
  char* strbuf = (char*) calloc (sizeof (char), buflen);
  char* strend = strbuf;
  int stufflen = strlen (STUFF);

  if (!strbuf) {
    return -__LINE__;
    }

  for (i = 0; i < n; i++) {
    if ( ( (strbuf + buflen) - strend) < (stufflen + 1)) {
      buflen = 2 * buflen;
      strbuf = (char*) realloc (strbuf, buflen);

      if (!strbuf) {
        return -__LINE__;
        }

      strend = strbuf + strlen (strbuf);
      }

    strcat (strend, STUFF);
    strend += stufflen;
    }

  fprintf (stdout, "%d\n", strlen (strbuf));
  free (strbuf);

  return 0;
  }

/*
 *  bconcatTest is a simple rerendering of the above using the bstrlib
 *  library.  One might like to compare the code below to the Java source
 *  code for this same test.
 *
 *  This example demonstrates how well bstrlib interoperates with the standard
 *  C library functions (fprintf).  It also demonstrates how bstrlib's
 *  additional constant string semantics (bsStatic) allows it to dramatically
 *  outperform C for the simplistic string manipulation operation of
 *  concatenation.
 */
static int bconcatTest (int n) {
  bstring b = cstr2bstr ("");
  struct tagbstring bstrSTUFF = bsStatic (STUFF);
  int i;

  if (!b) {
    return -__LINE__;
    }

  for (i = 0; i < n; i++) {
    if (0 > bconcat (b, &bstrSTUFF)) {
      return -__LINE__;
      }
    }

  fprintf (stdout, "%d\n", blength (b));
  bdestroy (b);
  return 0;
  }

int main (int argc, char* argv[]) {
  clock_t c0, c1;

  c0 = clock ();

  if (0 > strcatTest ( (argc == 2) ? atoi (argv[1]) : 1)) {
    fprintf (stderr, "Some kind of memory allocation error\n");
    return -1;
    }

  c1 = clock ();
  printf ("Time %.2fs\n", (c1 - c0) * (1.0 / (double) CLOCKS_PER_SEC));

  c0 = clock ();

  if (0 > bconcatTest ( (argc == 2) ? atoi (argv[1]) : 1)) {
    fprintf (stderr, "Some kind of memory allocation error\n");
    return -1;
    }

  c1 = clock ();
  printf ("Time %.2fs\n", (c1 - c0) * (1.0 / (double) CLOCKS_PER_SEC));

  return 0;
  }
