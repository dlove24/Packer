#include <stdio.h>
#include <stdlib.h>
#include "bstrlib.h"

/*
 *  In the usenet newsgroup comp.lang.c Kay wrote:
 *
 *    If i want to read data from a txt file,
 *    eg  John; 23; a
 *        Mary; 16; i
 *    How can I read the above data stopping reading before each semi-colon
 *    and save it in three different [entries of a node in a linked list]?
 *
 *  Below is a demonstration of how this problem is solved using the bsplit*cb
 *  scanning function in a nested manner.
 */

/* Linked list node */
struct entries {
  struct entries* next;
  bstring name;
  int age;
  bstring extra;
  };

struct cbsColum {
  /* Parse state */
  bstring src;
  int col;

  /* Output */
  struct tagbstring name;
  int age;
  struct tagbstring extra;
  };

static int columnCb (void* parm, int ofs, int len) {
  struct cbsColum* sc = (struct cbsColum*) parm;
  int ret;

  switch (sc->col) {
    case 0: /* Name */
      blk2tbstr (sc->name, sc->src->data + ofs, len);
      break;

    case 1: /* Age */
      ret = sscanf ( (char*) sc->src->data + ofs, "%d", &sc->age);

      if (ret != 1) {
        return -__LINE__;
        }

      break;

    case 2: /* Extra info */

      while (len > 0 && sc->src->data[ofs] == ' ') {
        ofs ++;
        len --;
        }

      blk2tbstr (sc->extra, sc->src->data + ofs, len);
      break;

    default:
      return -__LINE__;
    }

  sc->col++;
  return 0;
  }

struct cbsScanLine {
  /* Parse state */
  bstring src;
  int row;

  /* Output */
  struct entries** ll;
  };

static int lineCb (void* parm, int ofs, int len) {
  struct cbsScanLine* sl = (struct cbsScanLine*) parm;
  struct cbsColum sc;
  struct tagbstring t;
  int ret;

  /* Ignore empty lines and empty space between \r and \n */
  if (len == 0) {
    return 0;
    }

  blk2tbstr (t, sl->src->data + ofs, len);
  sc.src = &t;
  sc.col = 0;
  sl->row++;

  /* Split the input into individual entries */
  ret = bsplitcb (&t, (char) ';', 0, columnCb, &sc);

  if (0 == ret) {
    if (sc.col < 3) {
      return -__LINE__;
      }

    *sl->ll = (struct entries*) malloc (sizeof (struct entries));
    (*sl->ll)->name  = bstrcpy (&sc.name);
    (*sl->ll)->age   = sc.age;
    (*sl->ll)->extra = bstrcpy (&sc.extra);
    (*sl->ll)->next  = NULL;
    sl->ll = & ( (*sl->ll)->next);
    }

  return ret;
  }

/* Line seperators. */
static struct tagbstring crlf = bsStatic ("\r\n");

static int parseLines (const bstring src) {
  struct cbsScanLine sl;
  struct entries* ll, * x;
  int ret;

  sl.src = src;
  sl.row = 0;
  sl.ll = &ll;
  ll = NULL;

  /* Split the input into lines (rows) */
  ret =  bsplitscb (src, &crlf, 0, lineCb, &sl);

  if (0 == ret) {
    /* Output the entries of the linked list */
    for (x = ll; x != NULL;) {
      printf ("<%s> <%d> <%s>\n", bdatae (x->name, "NULL"), x->age, bdatae (x->extra, "NULL"));
      x = x->next;
      }

    /* Could "loop-jam" this with above, but its left seperate
       to make it clear */
    for (x = ll; x != NULL;) {
      bdestroy (x->name);
      bdestroy (x->extra);
      x = x->next;
      free (ll);
      ll = x;
      }
    }

  return ret;
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

