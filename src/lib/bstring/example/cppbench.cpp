#include <stdio.h>
#include <time.h>
#include "bstrlib.h"
#include "bstrwrap.h"
#include "bstraux.h"

/*
 *  Benchmark based on the features tested by benchmark seen here:
 *  http://www.utilitycode.com/str/performance.aspx
 *
 *  The idea is to measure the performance empty constructors, char *
 *  constructors, assignment, concatenation and scanning.  But rather than
 *  comparing MFC's CString to Str Library, we are going to compare STL to
 *  CBString.
 *
 *  The results are interesting.  CBString basically beats std::string quite
 *  severely, except for the empty constructor.
 */
#if defined (BSTRLIB_CAN_USE_STL)
#include <string>
#endif

#define TEST_SECONDS (5)

int timeTest (double& res, int (*testfn) (int count), int count) {
  clock_t t0, t1;
  int p = 0;
  double x, c = 0;

  t0 = clock ();

  do {
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
  while (1);

  res = c * CLOCKS_PER_SEC / (t1 - t0);
  return p;
  }

#define TESTSTRING1 ("<sometag name=\"John Doe\" position=\"Executive VP Marketing\"/>")

#if defined (BSTRLIB_CAN_USE_STL)
int testSTL_emptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    std::string b;
    c += b.length ();
    }

  return c;
  }

int testSTL_nonemptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    std::string b (TESTSTRING1);
    c += b.length ();
    }

  return c;
  }

int testSTL_cstrAssignment (int count) {
  int i, c = 0;
  std::string b;

  for (c = i = 0; i < count; i++) {
    b = TESTSTRING1;
    c += b.length ();
    }

  return c;
  }

int testSTL_extraction (int count) {
  int i, c = 0;
  std::string b (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += b[ (i & 7)];
    c += b[ (i & 7) ^ 8];
    c += b.c_str() [ (i & 7) ^ 4];
    }

  return c;
  }

int testSTL_scan (int count) {
  int i, c = 0;
  std::string b ("Dot. 123. Some more data.");

  for (c = i = 0; i < count; i++) {
    c += b.find ('.');
    c += b.find ("123");
    c += b.find_first_of ("sm");
    }

  return c;
  }

int testSTL_concat (int count) {
  int i, j, c = 0;
  std::string a (TESTSTRING1);
  std::string accum;

  for (j = 0; j < count; j++) {
    accum = "";

    for (i = 0; i < 250; i++) {
      accum += a;
      accum += "!!";
      }
    }

  return c;
  }

int testSTL_replace (int count) {
  int j, c = 0;
  std::string a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    a.replace (11, 4, "XXXXXX");
    a.replace (23, 2, "XXXXXX");
    a.replace (4, 8, "XX");
    c += a.length ();
    }

  return c;
  }

#endif

int testCBS_emptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    CBString b;
    c += b.length ();
    }

  return c;
  }

int testCBS_nonemptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    CBString b (TESTSTRING1);
    c += b.length ();
    }

  return c;
  }

int testCBS_cstrAssignment (int count) {
  int i, c = 0;
  CBString b;

  for (c = i = 0; i < count; i++) {
    b = TESTSTRING1;
    c += b.length ();
    }

  return c;
  }

int testCBS_extraction (int count) {
  int i, c = 0;
  CBString b (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += b[ (i & 7)];
    c += b[ (i & 7) ^ 8];
    c += ( (const char*) b) [ (i & 7) ^ 4];
    }

  return c;
  }

int testCBS_scan (int count) {
  int i, c = 0;
  CBString b ("Dot. 123. Some more data.");

  for (c = i = 0; i < count; i++) {
    c += b.find ('.');
    c += b.find ("123");
    c += b.findchr ("sm");
    }

  return c;
  }

int testCBS_concat (int count) {
  int i, j, c = 0;
  CBString a (TESTSTRING1);
  CBString accum;

  for (j = 0; j < count; j++) {
    accum = "";

    for (i = 0; i < 250; i++) {
      accum += a;
      accum += "!!";
      }
    }

  return c;
  }

int testCBS_replace (int count) {
  int j, c = 0;
  CBString a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    a.replace (11, 4, "XXXXXX");
    a.replace (23, 2, "XXXXXX");
    a.replace (4, 8, "XX");
    c += a.length ();
    }

  return c;
  }

#define NTESTS 7
struct flags {
  int runtest[NTESTS];
  };

int benchTest (const struct flags* runflags) {
  int c = 0;
  double cps;

#if defined (BSTRLIB_CAN_USE_STL)

  if (runflags->runtest[0]) {
    c += timeTest (cps, testSTL_emptyCtor, 100000);
    printf ("std::string empty constructor:     %20.1f per second\n", cps);
    }

  if (runflags->runtest[1]) {
    c += timeTest (cps, testSTL_nonemptyCtor, 100000);
    printf ("std::string non-empty constructor: %20.1f per second\n", cps);
    }

  if (runflags->runtest[2]) {
    c += timeTest (cps, testSTL_cstrAssignment, 100000);
    printf ("std::string char * assignment:     %20.1f per second\n", cps);
    }

  if (runflags->runtest[3]) {
    c += timeTest (cps, testSTL_extraction, 100000);
    printf ("std::string char extraction:       %20.1f per second\n", cps);
    }

  if (runflags->runtest[4]) {
    c += timeTest (cps, testSTL_scan, 100000);
    printf ("std::string scan:                  %20.1f per second\n", cps);
    }

  if (runflags->runtest[5]) {
    c += timeTest (cps, testSTL_concat, 10);
    printf ("std::string concatenation:         %20.1f per second\n", cps * 250);
    }

  if (runflags->runtest[6]) {
    c += timeTest (cps, testSTL_replace, 10000);
    printf ("std::string replace:               %20.1f per second\n", cps);
    }

#endif

  if (runflags->runtest[0]) {
    c += timeTest (cps, testCBS_emptyCtor, 100000);
    printf ("CBString empty constructor:        %20.1f per second\n", cps);
    }

  if (runflags->runtest[1]) {
    c += timeTest (cps, testCBS_nonemptyCtor, 100000);
    printf ("CBString non-empty constructor:    %20.1f per second\n", cps);
    }

  if (runflags->runtest[2]) {
    c += timeTest (cps, testCBS_cstrAssignment, 100000);
    printf ("CBString char * assignment:        %20.1f per second\n", cps);
    }

  if (runflags->runtest[3]) {
    c += timeTest (cps, testCBS_extraction, 100000);
    printf ("CBString char extraction:          %20.1f per second\n", cps);
    }

  if (runflags->runtest[4]) {
    c += timeTest (cps, testCBS_scan, 100000);
    printf ("CBString scan:                     %20.1f per second\n", cps);
    }

  if (runflags->runtest[5]) {
    c += timeTest (cps, testCBS_concat, 10);
    printf ("CBString concatenation:            %20.1f per second\n", cps * 250);
    }

  if (runflags->runtest[6]) {
    c += timeTest (cps, testCBS_replace, 10000);
    printf ("CBString replace:                  %20.1f per second\n", cps);
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
