#include <stdio.h>
#include "bstrlib.h"

/*
 *  In the usenet newsgroups comp.lang.c and comp.lang.c++ Kai Jaensch wrote:
 *
 *    [...] I have a .txt-file in which the data is structured in that way:
 *    Project-Nr. ID name  lastname
 *    33  9  Lars     Lundel
 *    33  12 Emil     Korla
 *    34  19 Lara     Keuler
 *    33  13 Thorsten Lammert
 *
 *    These data have to be read out row by row.
 *    Every row has to be splitted (delimiter is TAB) and has to be saved in
 *    an two-dimensional array.
 *
 *  Below is a demonstration of how this problem is solved using the bsplitcb
 *  scanning function in a nested manner.
 */

struct cbsScanLine {
  bstring src;
  int row, col;
  };

static int columnCb (void* parm, int ofs, int len) {
  struct cbsScanLine* sc = (struct cbsScanLine*) parm;
  bstring s;
  struct tagbstring t;

  if (len == 0) {
    return 0;
    }

  blk2tbstr (t, sc->src->data + ofs, len);
  sc->col++;

  s = bstrcpy (&t);

  /* Actually the following line would would be substituted by code
     which stored the result in the final array.  However, given the
     issue of possible buffer overflow, I leave the details to the
     reader. */
  printf ("[%2d,%2d] -> <%s>\n", sc->row, sc->col, s->data);
  bdestroy (s);
  return 0;
  }

/* The spacing delimiters (here I included both tab and space.) */
static struct tagbstring ws = bsStatic ("\t ");

static int lineCb (void* parm, int ofs, int len) {
  struct cbsScanLine* sl = (struct cbsScanLine*) parm;
  struct cbsScanLine sc;
  struct tagbstring t;

  if (len > 0 && sl->src->data[ofs + len - 1] == '\r') {
    len--;
    }

  blk2tbstr (t, sl->src->data + ofs, len);
  sl->row++;
  sc.src = &t;
  sc.row = sl->row;
  sc.col = 0;

  /* Split the input into individual entries */
  return bsplitscb (&t, &ws, 0, columnCb, &sc);
  }

static int parseLines (const bstring src) {
  struct cbsScanLine sl;
  sl.src = src;
  sl.row = sl.col = 0;

  /* Split the input into lines (rows) */
  return bsplitcb (src, (char) '\n', 0, lineCb, &sl);
  }

int main (int argc, char* argv[]) {
  FILE* fp;

  if (argc < 2) {
    printf ("%s [inputfile]\n", argv[0]);
    return -__LINE__;
    }

  if (NULL != (fp = fopen (argv[1], "rb"))) {
    bstring src = bread ( (bNread) fread, fp);
    int ret = parseLines (src);
    fclose (fp);
    bdestroy (src);
    return ret;
    }

  return -__LINE__;
  }

