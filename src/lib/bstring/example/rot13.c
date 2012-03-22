#include <stdio.h>
#include <limits.h>
#include "bstrlib.h"

static char rot13c[UCHAR_MAX + 1];

static size_t rot13read (void* buff, size_t elsize, size_t nelem, void* parm) {
  size_t i, ret = fread (buff, elsize, nelem, (FILE*) parm);

  for (i = 0; i < ret; i++) {
    ( (unsigned char*) buff) [i] = rot13c[ ( (unsigned char*) buff) [i]];
    }

  return ret;
  }

int main (int argc, char* argv[]) {
  FILE* fp = stdin;
  struct bStream* stream;
  int i;

  if (argc >= 2 && NULL == (fp = fopen (argv[1], "r"))) {
    fprintf (stderr, "Unable to read %s\n", argv[1]);
    return -__LINE__;
    }

  for (i = 0; i <= UCHAR_MAX; i++) {
    rot13c[i] = (unsigned char) i;

    if ( ('A' <= i && i <= 'M') || ('a' <= i && i <= 'm')) {
      rot13c[i] = (unsigned char) (i + 13);
      }

    if ( ('N' <= i && i <= 'Z') || ('n' <= i && i <= 'z')) {
      rot13c[i] = (unsigned char) (i - 13);
      }
    }

  if (NULL == (stream = bsopen ( (bNread) rot13read, fp))) {
    if (stdin != fp) {
      fclose (fp);
      }

    fprintf (stderr, "Unable to read %s\n", argv[1]);
    return -__LINE__;
    }

  else {
    bstring src = bfromcstr ("");

    while (BSTR_OK == bsread (src, stream, 256)) {
      fputs ( (char*) src->data, stdout);
      }

    bdestroy (src);
    bsclose (stream);
    }

  return 0;
  }

