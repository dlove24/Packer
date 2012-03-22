// mfcbench.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <time.h>
#include "bstrlib.h"
#include "bstrwrap.h"
#include "stdafx.h"

/*
 *  Benchmark based on the features tested by benchmark seen here:
 *  http://www.utilitycode.com/str/performance.aspx
 *
 *  The idea is to measure the performance empty constructors, char *
 *  constructors, assignment, concatenation and scanning.
 */

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

int testMFC_emptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    CString b;
    c += b.GetLength ();
    }

  return c;
  }

int testMFC_nonemptyCtor (int count) {
  int i, c = 0;

  for (c = i = 0; i < count; i++) {
    CString b (TESTSTRING1);
    c += b.GetLength ();
    }

  return c;
  }

int testMFC_cstrAssignment (int count) {
  int i, c = 0;
  CString b;

  for (c = i = 0; i < count; i++) {
    b = TESTSTRING1;
    c += b.GetLength ();
    }

  return c;
  }

int testMFC_extraction (int count) {
  int i, c = 0;
  CString b (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += b[ (i & 7)];
    c += b[ (i & 7) ^ 8];
    c += b[ (i & 7) ^ 4];
    }

  return c;
  }

int testMFC_unsafeextraction (int count) {
  int i, c = 0;
  CString b (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += ( (const char*) b) [ (i & 7)];
    c += ( (const char*) b) [ (i & 7) ^ 8];
    c += ( (const char*) b) [ (i & 7) ^ 4];
    }

  return c;
  }

int testMFC_scan (int count) {
  int i, c = 0;
  CString b ("Dot. 123. Some more data.");

  for (c = i = 0; i < count; i++) {
    c += b.Find ('.');
    c += b.Find ("123");
    c += b.FindOneOf ("sm");
    }

  return c;
  }

int testMFC_concat (int count) {
  int i, j, c = 0;
  CString accum, a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    accum = "";

    for (i = 0; i < 250; i++) {
      accum += a;
      accum += "!!";
      }
    }

  return c;
  }

int testMFC_replace (int count) {
  int j, c = 0;
  CString a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    a.Delete (11, 4);
    a.Insert (11, "XXXXXX");
    a.Delete (23, 2);
    a.Insert (23, "XXXXXX");
    a.Delete (4, 8);
    a.Insert (4, "XX");
    c += a.GetLength ();
    }

  return c;
  }

int testMFC_findreplace (int count) {
  int j, c = 0;
  CString a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    a.Replace ("e", "@");
    a.Replace ("@", "###");
    a.Replace ("###", "e");
    c += a.GetLength ();
    }

  return c;
  }

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
    c += b[ (i & 7) ^ 4];
    }

  return c;
  }

int testCBS_unsafeextraction (int count) {
  int i, c = 0;
  CBString b (TESTSTRING1);

  for (c = i = 0; i < count; i++) {
    c += ( (const char*) b) [ (i & 7)];
    c += ( (const char*) b) [ (i & 7) ^ 8];
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

int testCBS_findreplace (int count) {
  int j, c = 0;
  CBString a (TESTSTRING1);

  for (j = 0; j < count; j++) {
    a.findreplace ("e", "@");
    a.findreplace ("@", "###");
    a.findreplace ("###", "e");
    c += a.length ();
    }

  return c;
  }

void benchTest (void) {
  int c = 0;
  double cps;

  c += timeTest (cps, testMFC_emptyCtor, 100000);
  printf ("CString empty constructor:       %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_nonemptyCtor, 100000);
  printf ("CString non-empty constructor:   %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_cstrAssignment, 100000);
  printf ("CString char * assignment:       %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_extraction, 100000);
  printf ("CString char extraction:         %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_unsafeextraction, 100000);
  printf ("CString unsafe char extraction:  %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_scan, 100000);
  printf ("CString scan:                    %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_concat, 10);
  printf ("CString concatenation:           %20.1f per second\n", cps * 250);
  c += timeTest (cps, testMFC_replace, 10000);
  printf ("CString replace:                 %20.1f per second\n", cps);
  c += timeTest (cps, testMFC_findreplace, 10000);
  printf ("CString find and replace:        %20.1f per second\n", cps);

  c += timeTest (cps, testCBS_emptyCtor, 100000);
  printf ("CBString empty constructor:      %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_nonemptyCtor, 100000);
  printf ("CBString non-empty constructor:  %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_cstrAssignment, 100000);
  printf ("CBString char * assignment:      %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_extraction, 100000);
  printf ("CBString char extraction:        %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_unsafeextraction, 100000);
  printf ("CBString unsafe char extraction: %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_scan, 100000);
  printf ("CBString scan:                   %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_concat, 10);
  printf ("CBString concatenation:          %20.1f per second\n", cps * 250);
  c += timeTest (cps, testCBS_replace, 10000);
  printf ("CBString replace:                %20.1f per second\n", cps);
  c += timeTest (cps, testCBS_findreplace, 10000);
  printf ("CBString find and replace:       %20.1f per second\n", cps);

  return;
  }

int _tmain (int argc, TCHAR* argv[], TCHAR* envp[]) {
  argv = argv;
  argc = argc;
  envp = envp;

  benchTest ();
  return 0;
  }

