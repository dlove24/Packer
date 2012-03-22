#include <stdio.h>

#define PCRE_STATIC
#include "pcre.h"
#include "bstrlib.h"

/*
 *  An example demonstrating that interoperating with the PCRE (Perl
 *  Compatible Regular Expressions) library is straightforward.  It requires
 *  linking against bstrlib and the pcre library (for the Windows DLL version
 *  remove the #define PCRE_STATIC above).  For more information on using and
 *  obtaining PCRE see http://www.pcre.org
 */

int main (int argc, char* argv[]) {
  FILE* fp;

  if (argc < 3) {
    fprintf (stderr, "%s regex inputfilename\n", argv[0]);
    return -1;
    }

  if (NULL != (fp = fopen (argv[2], "r"))) {
    pcre* re;
    const char* errptr;
    int errofs;
    bstring b = bread ( (bNread) fread, fp);
    fclose (fp);

    if (b) {
      if (NULL != (re = pcre_compile (argv[1], PCRE_MULTILINE, &errptr, &errofs, NULL))) {
#       define VEC_LEN (64)
        int vec[VEC_LEN];
        int ret = pcre_exec (re, NULL, (const char*) b->data, blength (b), 0, 0, vec, VEC_LEN);

        printf ("ret = %d\n", ret);

        if (ret > 0) {
          int i, len;

          for (i = 0; i < ret; i++) {
            const char* c;
            len = pcre_get_substring ( (const char*) b->data, vec, ret, i, &c);

            if (len >= 0) {
              printf ("\t<%s>\n", c);
              }

            pcre_free_substring (c);
            }
          }

        pcre_free (re);
        }

      else {
        fprintf (stderr, "Bad RE: %s (offset = %d)\n", errptr, errofs);
        }

      bdestroy (b);
      }
    }

  else {
    fprintf (stderr, "Unable to open: %s\n", argv[2]);
    }

  return 0;
  }

