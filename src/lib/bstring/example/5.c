/*
 * 5: parse simple name/value pairs
 *
 * SAMPLE BUILD:
 * cc -g -Wall -o 5 5.c ../bstrlib.c -I../
 *
 * SAMPLE OUTPUT:
 * ./5 =>
 *   Initializing parameters to default values...
 *   Reading config file...
 *   Final values:
 *     item: cone, flavor: vanilla, size: large
 */
#include <stdio.h>
#include "bstrlib.h"

#define DEFAULT_CONFIG_FILE "sample.txt"

struct sample_parameters {
  bstring item;
  bstring flavor;
  bstring size;
  };

/*
 * initialize data to default values
 */
static void
init_parameters (struct sample_parameters* parms) {
  parms->item   = bfromcstr ("cup");
  parms->flavor = bfromcstr ("chocolate");
  parms->size   = bfromcstr ("small");
  }

/*
 * parse external parameters file
 *
 * returns 0 if completely successful.
 * returns a positive integer if there are parsing warnings.
 * returns a negative integer if a more catastrophic error occurrs.
 */
static int
parse_config (struct sample_parameters* parms, const char* filename) {
  FILE* fp = fopen (filename, "r");
  bstring line, b;
  int i, warnings;

  if (fp == NULL) {
    return -__LINE__;
    }

  for (warnings = 0;
       NULL != (line = bgets ( (bNgetc) fgetc, fp, (char) '\n'));
       bdestroy (line)) {

    btrimws (line); /* Remove extraneous whitespace & trailing \n */

    if (0 > (i = bstrchr (line, '='))) { /* Search for = character */
      if (blength (line)) {
        fprintf (stderr, "Syntax error: in %s\n"
                 "Should contain \"<var>=<parm>\"\n", line->data);
        warnings++;
        }

      continue;
      }

    b = bmidstr (line, 0, i); /* everything left of the = */
    btrimws (b);

    /* Ok, so which key is it? */

    if (biseqcstr (b, "item")) {
      bassignmidstr (parms->item, line, i + 1, blength (line));
      btrimws (parms->item);
      }

    else if (biseqcstr (b, "flavor")) {
      bassignmidstr (parms->flavor, line, i + 1, blength (line));
      btrimws (parms->flavor);
      }

    else if (biseqcstr (b, "size")) {
      bassignmidstr (parms->size, line, i + 1, blength (line));
      btrimws (parms->size);
      }

    else {
      char* tmp;
      fprintf (stderr, "%s is an unrecognized configuration key\n",
               tmp = bstr2cstr (b, (char) '\0'));
      bcstrfree (tmp);
      warnings++;
      }

    bdestroy (b);

    }

  fclose (fp);
  return 0;
  }

/*
 * program main
 */
int
main (int argc, char* argv[]) {
  struct sample_parameters parms;

  printf ("Initializing parameters to default values...\n");
  init_parameters (&parms);

  printf ("Reading config file...\n");
  parse_config (&parms, (argc > 1) ? argv[1] : DEFAULT_CONFIG_FILE);

  printf ("Final values:\n"
          "  item: %s, flavor: %s, size: %s\n",
          bdatae (parms.item, "???"),
          bdatae (parms.flavor, "???"),
          bdatae (parms.size, "???"));

  return 0;
  }

