#include <stdio.h>
#include <time.h>
#include <string.h>
#include "bstrlib.h"
#include "bstraux.h"

/*
 *  Benchmark based on the features tested by cppbench.
 *
 *  The idea is to measure the performance empty constructors, char *
 *  constructors, assignment, concatenation, scanning and replace.
 */

#define TEST_SECONDS (2)

#ifndef CLOCKS_PER_SEC
# ifdef CLK_TCK
#  define CLOCKS_PER_SEC (CLK_TCK)
# endif
#endif

static int timeTest (double* res, int (*testfn) (int count), int count) {
  clock_t t0, t1;
  int p = 0;
  double x, c = 0;

  t0 = clock ();

  for (;;) {
    p += testfn (count);
    t1 = clock ();

    c += count;

    if (t1 == t0) {
      if (count < (INT_MAX / 2)) {
        count += count;
        }

      continue;
      }

    if (t1 - t0 >= TEST_SECONDS * CLOCKS_PER_SEC) {
      break;
      }

    x = (TEST_SECONDS * CLOCKS_PER_SEC - (t1 - t0)) * c / (t1 - t0);

    if (x > INT_MAX) {
      x = INT_MAX;
      }

    count = (int) x;

    if (count < 1000) {
      count = 1000;
      }

    }

  if (res) {
    *res = c * CLOCKS_PER_SEC / (t1 - t0);
    }

  return p;
  }

#define TESTSTRING1 "<sometag name=\"John Doe\" position=\"Executive VP Marketing\"/>"

static int testCSTR_cstrAssignment (int count) {
  int i, c = 0;
  char b0 [1 + sizeof TESTSTRING1];
  char b1 [1 + sizeof TESTSTRING1];
  char b2 [1 + sizeof TESTSTRING1];

  b1[0] = b2[0] = (char) '\0';

  for (i = 0; i < count; i++) {
    strcpy (b0, b1);
    strcpy (b1, b2);
    strcpy (b2, TESTSTRING1);
    c += b0[0];
    }

  return c;
  }

static int testCSTR_extraction (int count) {
  int i, c;
  char b [1 + sizeof TESTSTRING1];

  strcpy (b, TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += b[i & 7];
    c += b[ (i & 7) ^ 8];
    }

  return c;
  }

static int testCSTR_scan (int count) {
  int i, c;
  char* p;
  char b [1 + sizeof TESTSTRING1];

  strcpy (b, TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    if (NULL != (p = strchr (b, '.'))) {
      c += (int) (p - b);
      }

    if (NULL != (p = strstr (b, "123"))) {
      c += (int) (p - b);
      }

    c += strcspn (b, "sm");
    }

  return c;
  }

static int testCSTR_concat (int count) {
  int i, j;
  static char accum[ ( (sizeof TESTSTRING1) + 1) * 250];

  for (j = 0; j < count; j++) {
    accum[0] = (char) '\0';

    for (i = 0; i < 250; i++) {
      strcat (accum, TESTSTRING1);
      strcat (accum, "!!");
      }
    }

  return 0;
  }

static int testCSTR_replace (int count) {
  int j;
  char a[1 + sizeof (TESTSTRING1) + 32];

  strcpy (a, TESTSTRING1);

  for (j = 0; j < count; j++) {
    memmove (a + 11 + 6, a + 11 + 4, strlen (a + 11 + 4) + 1);
    memcpy (a + 11, "XXXXXX", 6);

    memmove (a + 23 + 6, a + 23 + 2, strlen (a + 23 + 2) + 1);
    memcpy (a + 23, "XXXXXX", 6);

    memmove (a +  4 + 2, a +  4 + 8, strlen (a +  4 + 8) + 1);
    memcpy (a +  4, "XX", 2);
    }

  return 0;
  }

static int testBSTR_cstrAssignment (int count) {
  int i, c;
  bstring b0 = bfromcstr ("");
  bstring b1 = bfromcstr ("");
  bstring b2 = bfromcstr ("");
  struct tagbstring t = bsStatic (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    bassign (b0, b1);
    bassign (b1, b2);
    bassign (b2, &t);
    c += b0->data[0];
    }

  bdestroy (b0);
  bdestroy (b1);
  bdestroy (b2);
  return c;
  }

static int testBSTR_extraction (int count) {
  int i, c;
  bstring b = bfromcstr (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += b->data[i & 7];
    c += b->data[ (i & 7) ^ 8];
    }

  bdestroy (b);
  return c;
  }

static int testBSTR_scan (int count) {
  int i, c;
  bstring b = bfromcstr ("Dot. 123. Some more data.");
  struct tagbstring t1 = bsStatic ("123");
  struct tagbstring t2 = bsStatic ("sm");

  for (c = i = 0; i < count; i++) {
    c += bstrchr (b, '.');
    c += binstr (b, 0, &t1);
    c += binchr (b, 0, &t2);
    }

  bdestroy (b);
  return c;
  }

static int testBSTR_concat (int count) {
  int i, j, c = 0;
  struct tagbstring a = bsStatic (TESTSTRING1);
  bstring accum = bfromcstr ("");

  if (accum == NULL) {
    fprintf (stderr, "Out of memory error");
    return -1;
    }

  for (j = 0; j < count; j++) {
    accum->slen = 0;

    for (i = 0; i < 250; i++) {
      bconcat (accum, &a);
      bcatcstr (accum, "!!");
      }
    }

  bdestroy (accum);
  return c;
  }

static int testBSTR_replace (int count) {
  int j;
  bstring a = bfromcstr (TESTSTRING1);
  struct tagbstring t0 = bsStatic ("XXXXXX");
  struct tagbstring t1 = bsStatic ("XX");

  for (j = 0; j < count; j++) {
    breplace (a, 11, 4, &t0, (char) '?');
    breplace (a, 23, 2, &t0, (char) '?');
    breplace (a,  4, 8, &t1, (char) '?');
    }

  bdestroy (a);
  return 0;
  }

#define NTESTS 5
struct flags {
  int runtest[NTESTS];
  };

static int benchTest (const struct flags* runflags) {
  int c = 0;
  double cps;

  if (runflags->runtest[0]) {
    c += timeTest (&cps, testCSTR_cstrAssignment, 100000);
    printf ("char * assignment:             %20.1f per second\n", cps);
    }

  if (runflags->runtest[1]) {
    c += timeTest (&cps, testCSTR_extraction, 100000);
    printf ("char * char extraction:        %20.1f per second\n", cps);
    }

  if (runflags->runtest[2]) {
    c += timeTest (&cps, testCSTR_scan, 100000);
    printf ("char * scan:                   %20.1f per second\n", cps);
    }

  if (runflags->runtest[3]) {
    c += timeTest (&cps, testCSTR_concat, 10);
    printf ("char * concatenation:          %20.1f per second\n", cps * 250);
    }

  if (runflags->runtest[4]) {
    c += timeTest (&cps, testCSTR_replace, 10000);
    printf ("char * replace:                %20.1f per second\n", cps);
    }

  if (runflags->runtest[0]) {
    c += timeTest (&cps, testBSTR_cstrAssignment, 100000);
    printf ("bstring assignment:            %20.1f per second\n", cps);
    }

  if (runflags->runtest[1]) {
    c += timeTest (&cps, testBSTR_extraction, 100000);
    printf ("bstring char extraction:       %20.1f per second\n", cps);
    }

  if (runflags->runtest[2]) {
    c += timeTest (&cps, testBSTR_scan, 100000);
    printf ("bstring scan:                  %20.1f per second\n", cps);
    }

  if (runflags->runtest[3]) {
    c += timeTest (&cps, testBSTR_concat, 10);
    printf ("bstring concatenation:         %20.1f per second\n", cps * 250);
    }

  if (runflags->runtest[4]) {
    c += timeTest (&cps, testBSTR_replace, 10000);
    printf ("bstring replace:               %20.1f per second\n", cps);
    }

  return c;
  }

int main (int argc, char* argv[]) {
  struct flags runflags;
  int i, j;

  for (i = 0; i < NTESTS; i++) {
    runflags.runtest[i] = argc < 2;
    }

  for (i = 1; i < argc; i++) {
    if (1 == sscanf (argv[i], "%d", &j)) {
      if (j >= 1 && j <= NTESTS) {
        runflags.runtest[j - 1] = 1;
        }
      }
    }

  benchTest (&runflags);
  return 0;
  }
