#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "bstrlib.h"

/*
 *  Demo program scans for printable ASCII strings in an arbitrary file.
 *  Program exercises the split callback functions on streams as well as
 *  a single fixed string.
 *
 *  Enter strings.exe --help to get syntax for the program.
 *
 *  Note this does not behave exactly like the UNIX/GNU command strings;
 *  it does not have special recognition or behavior for object files.
 */

bstring nonascii;

struct parms {
  int minLen;
  int showFilename;
  char* filename;
  char* pgmname;
  char* ofsfmt;
  struct bStream* stream;
  };

static int entry (void* parm, int ofs, const struct tagbstring* entry) {
  struct parms* p = (struct parms*) parm;

  if (entry->slen >= p->minLen) {
    if (p->ofsfmt) {
      printf (p->ofsfmt, ofs);
      }

    if (p->showFilename) {
      printf ("%s:", p->filename);
      }

    puts ( (char*) entry->data);
    }

  return 0;
  }

static void process (struct parms* p) {
  bssplitscb (p->stream, nonascii, entry, p);
  }

static int preprocess (void* parm, int ofs, int len) {
  struct parms lParms, * p = (struct parms*) parm;
  bstring b = blk2bstr (p->filename + ofs, len);
  FILE* fp;

  lParms = *p;
  lParms.filename = (char*) b->data;
  fp = fopen (lParms.filename, "rb");

  if (NULL == fp) {
    fprintf (stderr, "%s: '%s': No such file", lParms.pgmname, lParms.filename);
    bdestroy (b);
    return -__LINE__;
    }

  lParms.stream = bsopen ( (bNread) fread, fp);
  process (&lParms);
  bsclose (lParms.stream);
  fclose (fp);
  return 0;
  }

int main (int argc, char* argv[]) {
  int i;
  bstring infiles = NULL;
  struct parms cmdParms;

  cmdParms.minLen = 4;
  cmdParms.showFilename = 0;
  cmdParms.pgmname = argv[0];
  cmdParms.ofsfmt = NULL;

  for (i = 0; cmdParms.pgmname[i];) {
    if (cmdParms.pgmname[i] == '\\' ||
        cmdParms.pgmname[i] == ':'  ||
        cmdParms.pgmname[i] == '/') {
      cmdParms.pgmname += i + 1;
      i = 0;
      }

    else {
      i++;
      }
    }

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'f' || (argv[i][1] == '-' && argv[i][2] == 'p')) {
        cmdParms.showFilename = 1;
        }

      else if (argv[i][1] == 'n') {
        char* ml;

        if (argv[i][2] == '\0') {
          i++;

          if (i >= argc) {
            goto Usage;
            }

          ml = argv[i];
          }

        else {
          ml = argv[i] + 2;
          }

        if (1 != sscanf (ml, "%d", &cmdParms.minLen)) {
          fprintf (stderr, "strings: invalid integer argument %s\n", argv[i]);
          exit (EXIT_FAILURE);
          }
        }

      else if (argv[i][1] == 't') {
        static char radix[16];
        char* ml;

        if (argv[i][2] == '\0') {
          i++;

          if (i >= argc) {
            goto Usage;
            }

          ml = argv[i];
          }

        else {
          ml = argv[i] + 2;
          }

        if (ml[0] == '\0' || ml[1] != '\0') {
          goto Usage;
          }

        switch (ml[0]) {
          default:
            goto Usage;

          case 'o':
          case 'x':
          case 'd':
            sprintf (radix, "%%7%c ", ml[0]);
            cmdParms.ofsfmt = radix;
          }
        }

      else if (argv[i][1] == 'h' || (argv[i][1] == '-' && argv[i][2] == 'h')) {
Usage:
        ;
        fprintf (stderr, "Usage: %s [option(s)] [file(s)]\n", cmdParms.pgmname);
        fprintf (stderr, " Display the printable strings in [file(s)] (stdin by default)\n");
        fprintf (stderr, " The options are:\n");
        fprintf (stderr, "  -f --print-file-name      Print the name of the file before each string\n");
        fprintf (stderr, "  -n<number>                Locate & print any NUL-terminated sequence of at\n");
        fprintf (stderr, "                            least [number] characters (default 4).\n");
        fprintf (stderr, "  -t{o,x,d}                 Print the offset of the string in base 8, 10 or 16\n");
        fprintf (stderr, "  -h --help                 Display this information\n");
        exit (EXIT_FAILURE);
        }

      else {
        fprintf (stderr, "%s: invalid option -- %c\n", cmdParms.pgmname, argv[i][1]);
        goto Usage;
        }
      }

    else {
      if (NULL == infiles) {
        infiles = bfromcstr (argv[i]);
        }

      else {
        bconchar (infiles, (char) '\n');
        bcatcstr (infiles, argv[i]);
        }
      }
    }

  nonascii = bfromcstr ("");

  for (i = 0; i <= UCHAR_MAX; i++) {
    if (i < 32 || i > 128) {
      if (0 > bconchar (nonascii, (char) i)) {
        fprintf (stderr, "Out of memory");
        exit (EXIT_FAILURE);
        }
      }
    }

  if (NULL == infiles) {
    cmdParms.filename = "{standard input}";
    cmdParms.stream = bsopen ( (bNread) fread, stdin);
    process (&cmdParms);
    bsclose (cmdParms.stream);
    }

  else {
    cmdParms.filename = (char*) infiles->data;
    bsplitcb (infiles, (char) '\n', 0, preprocess, &cmdParms);
    bdestroy (infiles);
    }

  bdestroy (nonascii);

  return 0;
  }
