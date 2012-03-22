/**
*** Copyright (c) 2012 David Love <d.love@shu.ac.uk>
***
*** Permission to use, copy, modify, and/or distribute this software for any
*** purpose with or without fee is hereby granted, provided that the above
*** copyright notice and this permission notice appear in all copies.
***
*** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*** WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*** MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*** ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*** WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*** ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*** OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
***
*** \file braid.c
*** \brief The core command line interface to Braid on Unix-like machines
***
*** \author David Love
*** \date Feb 2012
**/

/* Include the standard library */
#include <stdlib.h>

/* Include the POSIX path functions */
#include <libgen.h>

/* Include the bstring library */
#include "bstring/bstrlib.h"
#include "bstring/bstraux.h"

/* Option processing is done via argtable */
#include "argtable2.h"

/**
*** Main Loop. This should do very little other than parse the command
*** line and call the appropriate library function.
**/
int main (int argc, char** argv) {
  int index = 0;                    /*< Temporary index value */

  const char* progname = "braid";  /*< Set the name of the program */

  int parse_errors;                 /*< The number of parse errors encountered */

  int exit_code = 0;                /*< The final exit code returned to the caller on termination */

  bstring input_file;              /*< The name of the input file */
  bstring input_file_path;          /*< The full name and path of the input file */

  bstring output_file;              /*< The name of the output file */
  bstring output_file_path;         /*< The full name and path of the output file */

  /* Tell the argtable library how our options are set-up */
  struct arg_lit*  verb  = arg_lit0 ("v", "verbose", "show processing diagnostics");
  struct arg_lit*  help  = arg_lit0 (NULL, "help",        "print this help and exit");
  struct arg_lit*  vers  = arg_lit0 (NULL, "version",     "print version information and exit");
  struct arg_file* files = arg_filen (NULL, NULL, NULL, 1, argc + 2, NULL);
  struct arg_end*  end   = arg_end (20);

  void* argtable[5];
  argtable[0] = verb;
  argtable[1] = help;
  argtable[2] = vers;
  argtable[3] = files;
  argtable[4] = end;

  /* verify the argtable[] entries were allocated sucessfully */
  if (arg_nullcheck (argtable) != 0) {
    /* NULL entries were detected, some allocations must have failed */
    printf ("%s: insufficient memory\n", progname);

    exit_code = 1;
    goto call_exit;
    }

  /* Parse the command line as defined by argtable[] */
  parse_errors = arg_parse (argc, argv, argtable);

  /* special case: '--help' takes precedence over error reporting */
  if (help->count > 0) {
    printf ("Usage: %s", progname);
    arg_print_syntax (stdout, argtable, "\n");
    printf ("Remove (unlink) the specified file(s).\n\n");
    arg_print_glossary (stdout, argtable, "  %-20s %s\n");
    printf ("\nReport bugs to <no-one> as this is just an example program.\n");

    exit_code = 0;
    goto call_exit;
    }

  /* special case: '--version' takes precedence error reporting */
  if (vers->count > 0) {
    printf ("'%s' example program for the \"argtable\" command line argument parser.\n", progname);
    printf ("September 2003, Stewart Heitmann\n");

    exit_code = 0;
    goto call_exit;
    }

  /* If the parser returned any errors then display them and exit */
  if (parse_errors > 0) {

    arg_print_errors (stdout, end, progname);
    printf ("Invalid arguments. Try '%s --help' for more information.\n", progname);

    exit_code = 1;
    goto call_exit;
    }

  /* Count the number of file argument: we should have exactly two (one
   * for input and one for output). If we only have one file, assume
   * this file is the input, and form the output file from the input
   * file
   */
  if (files->count == 1) {
    /* We should have at least two files, so pick the first argument and
     * form the input and output file arguments from it
     */
    input_file_path = bfromcstr (files->filename[0]);

    if (!input_file_path) {
      fprintf (stderr, "Allocation of the input file path failed\n");
      exit_code = 10;
      goto call_exit;
      }

    output_file_path = bfromcstralloc (12, "");

    if (!output_file_path) {
      fprintf (stderr, "Allocation of the output file path failed\n");
      exit_code = 10;
      goto call_exit;
      }

    /* Work out the path for the output file
     *
     */

    bassigncstr (output_file_path, dirname (bdata (input_file_path)));

    /* Check for an empty directory in the path */
    if ( (blength (output_file_path) == 1) && (bchar (output_file_path, 0) == '.')) {
      /* If the path is should be empty, clear the contents of
       * the output_file_path
       */
      bassigncstr (output_file_path, "");
      }

    else {
      /* If we do have a file path, tack on the path deliminator
       * so that we can just add the file name when we have it
       */
      bconchar (output_file_path, '/');
      }

    if (!output_file_path) {
      fprintf (stderr, "Allocation of the output filename failed");
      exit_code = 10;
      goto call_exit;
      }

    /*
     * Form the name of the output file from the name of the
     * input file
     */

    input_file = bfromcstralloc (12, "");

    if (!input_file) {
      fprintf (stderr, "Cannot allocate space for input file name");
      exit_code = 10;
      goto call_exit;
      }

    output_file = bfromcstralloc (12, "");

    if (!output_file) {
      fprintf (stderr, "Cannot allocate space for output file name");
      exit_code = 10;
      goto call_exit;
      }

    bassigncstr (input_file, basename (bdata (input_file_path)));

    index = bstrrchr (input_file, '.');

    if (index != BSTR_ERR) {
      /* We have found the last period, so copy everything
       * up to the period from the +input_file+ into the
       * +output_file+
       */
      output_file = bstrcpy (bmidstr (input_file, 0, index));

      if (!output_file) {
        fprintf (stderr, "Construction of the output filename failed");
        exit_code = 10;
        goto call_exit;
        }

      /* Add the output extension */
      if (!bcatcstr (output_file, ".pdoc") == BSTR_OK) {
        fprintf (stderr, "An attempt to create the output filename failed");
        exit_code = 10;
        goto call_exit;
        }
      }

    else {
      /* The input doesn't look very well formed, so just tack
       * on 'output' and hope for the best
       */
      bassign (output_file, input_file);

      if (!bcatcstr (output_file, ".output") == BSTR_OK) {
        fprintf (stderr, "An attempt to create the output filename failed");
        exit_code = 10;
        goto call_exit;
        }
      }

    /* Form the final output path, using the current path and the
     * output_file_name
     */
    bconcat (output_file_path, output_file);

    }

  else {
    /* We should have at least two files, so pick the first two and
     * form the input and output file arguments from them
     */
    input_file_path = bfromcstr (files->filename[0]);
    output_file_path = bfromcstr (files->filename[1]);

    if (!input_file_path) {
      fprintf (stderr, "Allocation of the input filename failed");
      exit_code = 10;
      goto call_exit;
      }

    if (!output_file_path) {
      fprintf (stderr, "Allocation of the output filename failed");
      exit_code = 10;
      goto call_exit;
      }
    }

  /* If we have got here, we assume everything has been allocated
   * correctly, and all inputs validated. So we can now call the main
   * library, and hand control over
   */
  goto call_braid;

  /*
  ** Cleanup and terminate. Cleanup the environment, deallocate any memory
  ** and tell the operating system (caller) what happened via our exit code
  */

call_exit:

  /* Deallocate the memory used by argtable */
  arg_freetable (argtable, sizeof argtable / sizeof argtable[0]);

  /* Return the stated code to the caller */
  exit (exit_code);

call_braid:

  /* Deallocate the memory reserved by the options argtable */
  arg_freetable (argtable, sizeof argtable / sizeof argtable[0]);

  /* Call the main library */

  /* Deallocate the string library */
  bdestroy (input_file);
  bdestroy (output_file);
  bdestroy (input_file_path);
  bdestroy (output_file_path);

  /* Tell the caller eveyrthing went OK */
  exit (0);
  }
