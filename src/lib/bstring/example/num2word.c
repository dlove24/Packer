#include <stdio.h>
#include "bstrlib.h"

/*
 *  This is just a toy example which converts a number to a english or french
 *  text rendering of that number.  It should be fairly straight forward to
 *  see how to extend this to other languages (assuming it uses a similar
 *  decimal breakdown).
 */
struct ntabS {
  long val;
  struct tagbstring name;
  };

struct langS {
  struct ntabS ntabBig[4];
  struct ntabS ntab10s[8];
  struct tagbstring neg;
  struct tagbstring first20[20];
  };

static struct tagbstring errNum = bsStatic ("error");

static struct langS french = {
    { { 1000000000, bsStatic (" milliard") },
      {    1000000, bsStatic (" million") },
      {       1000, bsStatic (" mille") },
      {        100, bsStatic (" cent") }
    },

    { { 90, bsStatic ("quatre-vingt-dix") },
      { 80, bsStatic ("quatre-vingts") },
      { 70, bsStatic ("soixante-dix") },
      { 60, bsStatic ("soixante") },
      { 50, bsStatic ("cinquante") },
      { 40, bsStatic ("quarante") },
      { 30, bsStatic ("trente") },
      { 20, bsStatic ("vingt") }
    },

  bsStatic ("negatif "),

    {
    bsStatic ("zero"),
    bsStatic ("un"),
    bsStatic ("deux"),
    bsStatic ("trois"),
    bsStatic ("quatre"),
    bsStatic ("cinq"),
    bsStatic ("six"),
    bsStatic ("sept"),
    bsStatic ("huit"),
    bsStatic ("neuf"),
    bsStatic ("dix"),
    bsStatic ("onze"),
    bsStatic ("douze"),
    bsStatic ("treize"),
    bsStatic ("quatorze"),
    bsStatic ("quinze"),
    bsStatic ("seize"),
    bsStatic ("dix-sept"),
    bsStatic ("dix-huit"),
    bsStatic ("dix-neuf")
    }
  };

static struct langS english = {
    { { 1000000000, bsStatic (" billion") },
      {    1000000, bsStatic (" million") },
      {       1000, bsStatic (" thousand") },
      {        100, bsStatic (" hundred") }
    },

    { { 90, bsStatic ("ninety") },
      { 80, bsStatic ("eighty") },
      { 70, bsStatic ("seventy") },
      { 60, bsStatic ("sixty") },
      { 50, bsStatic ("fifty") },
      { 40, bsStatic ("forty") },
      { 30, bsStatic ("thirty") },
      { 20, bsStatic ("twenty") }
    },

  bsStatic ("negative "),

    {
    bsStatic ("zero"),
    bsStatic ("one"),
    bsStatic ("two"),
    bsStatic ("three"),
    bsStatic ("four"),
    bsStatic ("five"),
    bsStatic ("six"),
    bsStatic ("seven"),
    bsStatic ("eight"),
    bsStatic ("nine"),
    bsStatic ("ten"),
    bsStatic ("eleven"),
    bsStatic ("twelve"),
    bsStatic ("thirteen"),
    bsStatic ("forteen"),
    bsStatic ("fifteen"),
    bsStatic ("sixteen"),
    bsStatic ("seventeen"),
    bsStatic ("eighteen"),
    bsStatic ("nineteen")
    }
  };

/* Convert from long int to text description of the number */

static bstring word2Num (long int n, struct langS* lang) {
  bstring x, y;
  int i;

  /* negative numbers */
  if (n < 0) {
    if (n == -n) {
      /* The special number -2^31 cannot be negated in 2s
       * complement, so it has to be treated seperately. */
      x = word2Num (-2000000000, lang);
      y = word2Num (147483648, lang);
      bconchar (x, (char) ' ');
      bconcat (x, y);
      bdestroy (y);
      return x;
      }

    x = word2Num (-n, lang);
    binsert (x, 0, &lang->neg, (char) '?');
    return x;
    }

  /* 100+ */
  for (i = 0; i < 4; i++) {
    if (n >= lang->ntabBig[i].val) {
      bconcat (x = word2Num (n / lang->ntabBig[i].val, lang), &lang->ntabBig[i].name);

      if (0 != (n %= lang->ntabBig[i].val)) {
        bconchar (x, (char) ' ');
        bconcat (x, y = word2Num (n, lang));
        bdestroy (y);
        }

      return x;
      }
    }

  /* 20 - 99 */
  for (i = 0; i < 8; i++) {
    if (n >= lang->ntab10s[i].val) {
      x = bstrcpy (&lang->ntab10s[i].name);

      if (0 != (n -= lang->ntab10s[i].val)) {
        bconchar (x, (char) ' ');
        bconcat (x, y = word2Num (n, lang));
        bdestroy (y);
        }

      return x;
      }
    }

  if ( (unsigned) n >= 20) {
    return &errNum;
    }

  return bstrcpy (&lang->first20[n]);
  }

int main (int argc, char* argv[]) {
  int v;
  long n;
  struct langS* lang = &english;

  if (argc < 2) {
    v = scanf ("%ld", &n);
    }

  else {
    v = sscanf (argv[1], "%ld", &n);

    if (argc > 2) {
      if (argv[2][0] == 'f' || argv[2][0] == 'F') {
        lang = &french;
        }
      }
    }

  if (v != 1) {
    printf ("That's not a number\n");
    }

  else {
    bstring r = word2Num (n, lang);
    printf ("%s\n", (char*) r->data);
    bdestroy (r);
    }

  return v != 1;
  }

