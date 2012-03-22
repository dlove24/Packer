/* #define MAIN_TEST_FOR_CSV */

#ifdef MAIN_TEST_FOR_CSV
#include <stdio.h>
#include <time.h>
#endif

#include <stdlib.h>
#include <limits.h>
#include "bstrlib.h"
#include "csvparse.h"

/*

This module implements an incremental CSV parser.  Each entry is limited to
INT_MAX in length, however the total field-length or CSV file length is not
limited by any constraint.

CSV Grammar
===========

Note that the CSV file format is described in terms of octets which embeds
7-bit ASCII as subset of values expressable by each octet.  Such octets will
generally referred to as ascii-8 format.  So CSV does not have a direct
encoding in incompatible formats such as EBCDIC, UTF-16 or Base-64.  The
correctly parsable CSV grammar is described as follows:

  WSChar        = [ \v\f\t]
  NoWSChar      = <ascii-8 - [,"\r\n \v\f\t]>
  RawChar       = <ascii-8 - [,"\r\n]>
  NoQuoteChar   = <ascii-8 - ["]>
  ParadoxBug    = ["] WSChar* NoWSChar NoQuoteChar*
  QuotableText  = NoQuoteChar* (["] ["] NoQuoteChar* | ParadoxBug)*
  NonBlankEntry = NoWSChar (RawChar* NoWSChar)?        # Not bracketted by WS
                | ["] QuotableText ["]
  Entry         = []
                | NonBlankEntry
  Field         = WSChar*                              # 0 entries
                | WSChar* NonBlankEntry WSChar*        # 1 entry
                | WSChar* Entry WSChar* [,] Field      # 2 or more entries
  CSVFile       = Field
                | Field ([\r] [\n]? | [\n]) CSVFile

Legend:

- []       Represents the empty string.
- ascii-8  Represents one arbitrary octet.
- [ ... ]  Represents one character selected from the sequence of characters
           between the brackets.  Non-text characters are described in a C
           language character compatible way.  The [ character is represented
           as \[.
- <A - B>  Represents one octet from the set A that is not in the set B.
- A B      Represents the concatenation of rule A with the rule B.
- A*       Represents the rule A concatenated with itself 0 or more times.
- A?       Represents either the rule A or [].
- A | B    Represents either the rule A or the rule B.
- (A)      Represents the rule A taken as a whole.

Words which start with a capital letter are non-terminal rules.  Anything
after a # including the # is just a comment.

Notes:

- To accomodate for improperly formated quoted entries (such as is output by
  Borland's Paradox), any enclosed single quote which is not followed by any
  amount of white space then either a "," or carriage-return should be
  considered a stand-alone double quote.  Going forward, implementations
  should not output CSV data using the ParadoxBug rule.

- An input line which is just white space is 0 entries, not one single
  blank entry.  A single blank entry must be explicitely quoted.

Other documentation of CSV files use the word "record" instead of "field".

*/

/* Initialize the character set arrays for the various CSV grammar rules. */

static char parse_isWSChar[1 + UCHAR_MAX];

static void initWSChar (void) {
  int i;

  for (i = 0; i < 1 + UCHAR_MAX; i++) {
    parse_isWSChar[i] = (char) ( (' '  == i) ||
                                 ('\v' == i) ||
                                 ('\f' == i) ||
                                 ('\t' == i));
    }
  }

static char parse_isNoWSChar[1 + UCHAR_MAX];

static void initNoWSChar (void) {
  int i;

  for (i = 0; i < 1 + UCHAR_MAX; i++) {
    parse_isNoWSChar[i] = (char) ! ( (','  == i) ||
                                     ('"'  == i) ||
                                     ('\r' == i) ||
                                     ('\n' == i) ||
                                     (' '  == i) ||
                                     ('\v' == i) ||
                                     ('\f' == i) ||
                                     ('\t' == i));
    }
  }

static char parse_isRawChar[1 + UCHAR_MAX];

static void initRawChar (void) {
  int i;

  for (i = 0; i < 1 + UCHAR_MAX; i++) {
    parse_isRawChar[i] = (char) ( (','  != i) &&
                                  ('"'  != i) &&
                                  ('\r' != i) &&
                                  ('\n' != i));
    }
  }

static char parse_isNoQuoteChar[1 + UCHAR_MAX];

static void initNoQuoteChar (void) {
  int i;

  for (i = 0; i < 1 + UCHAR_MAX; i++) {
    parse_isNoQuoteChar[i] = (char) ('"' != i);
    }
  }

static int alreadyInitialized = 0;

static void initCSVParseCharsets (void) {
  if (!alreadyInitialized) {
    initWSChar ();
    initNoWSChar ();
    initRawChar ();
    initNoQuoteChar ();
    alreadyInitialized = 1;
    }
  }

/* A tape is a performance enhancement over struct bStreams with respect to
   the semantic of being able to move both forwards and backwards through a
   stream, as well as trivial charSet parsing.
*/

struct tape {
  struct bStream* s;
  bstring b;
  int ofs;
  };

#define tapeMoveHead(tape,delta) (tape)->ofs += (delta)

#define TAPE_READ_BLOCK (4096)

static int tapeGetNextChar (struct tape* tape) {
  do {
    if (tape->ofs < tape->b->slen) {
      int ret = tape->b->data[tape->ofs];
      tapeMoveHead (tape, 1);
      return ret;
      }
    }
  while (BSTR_ERR != bsreada (tape->b, tape->s, TAPE_READ_BLOCK));

  return -__LINE__;
  }

/* tape ends up pointing at first character not in the charSet starting from
   its initial starting point.  The number of characters in the charSet until
   that point is added to *i.  The value of the next character is returned,
   or a negative value if the end of tape has been reached. */

static int tapeScanLen (struct tape* tape, int* i, const char charSet[1 + UCHAR_MAX]) {
  int n = tape->ofs;

  do {
    if (tape->ofs < tape->b->slen) {
      int len = tape->b->slen;
      unsigned char* x = tape->b->data;

      for (; n < len; n++) { /* Performance critical path is here. */
        if (!charSet[x[n]]) {
          *i += n - tape->ofs;
          tape->ofs = n;
          return x[n];
          }
        }
      }
    }
  while (BSTR_ERR != bsreada (tape->b, tape->s, TAPE_READ_BLOCK));

  *i += tape->b->slen - tape->ofs;
  tape->ofs = tape->b->slen;
  return -__LINE__;
  }

#define TAPE_SQUASH_THRESHOLD (4096)

static int tapeSquashPrevious (struct tape* tape) {
  int ret;

  /* Don't bother if what we are holding is fairly small. */
  if (tape->b->slen < TAPE_SQUASH_THRESHOLD) {
    return 0;
    }

  /* We don't need anything before the current head, so we
     reduce memory usage here. */
  if (0 > (ret = bdelete (tape->b, 0, tape->ofs))) {
    return ret;
    }

  tape->ofs = 0;
  return 0;
  }

/* Break down according to the parsing rules. */

/* ParadoxBug    = ["] WSChar* NoWSChar NoQuoteChar* */
static int parseParadoxBug (int* len, struct tape* tape) {
  int c, i;

  /* ["] */
  if ('"' != tapeGetNextChar (tape)) {
    tapeMoveHead (tape, -1);
    return 0;
    }

  /* WSChar* */
  i = 1;
  c = tapeScanLen (tape, &i, parse_isWSChar);

  /* NoWSChar */
  if (0 > c || !parse_isNoWSChar[c]) {
    tapeMoveHead (tape, -i);
    return 0;
    }

  tapeMoveHead (tape, 1);
  i++;

  /* NoQuoteChar* */
  c = tapeScanLen (tape, &i, parse_isNoQuoteChar);

  if (len) {
    *len = i;
    }

  return 1;
  }

/* QuotableText  = NoQuoteChar* (["] ["] NoQuoteChar* | ParadoxBug)* */
static int parseQuotableText (int* len, struct tape* tape) {
  int c, c1, i, j;

  /* NoQuoteChar* */
  i = 0;
  c = tapeScanLen (tape, &i, parse_isNoQuoteChar);

L1:
  ;

  if (0 > c) {
    if (len) {
      *len = i;
      }

    return 1;
    }

  /* (["] ["] NoQuoteChar* | ParadoxBug)* */
  if ('"' == c) {
    tapeMoveHead (tape, 1);
    c1 = tapeGetNextChar (tape);

    /* ["] ["] */
    if ('"' == c1) {
      i += 2;

      /* NoQuoteChar* */
      c = tapeScanLen (tape, &i, parse_isNoQuoteChar);
      goto L1;
      }

    if (0 > c1) {
      tapeMoveHead (tape, -1);
      c = c1;
      goto L1;
      }

    /* |ParadoxBug */
    tapeMoveHead (tape, -2);

    if (0 < parseParadoxBug (&j, tape)) {
      i += j;
      c = tapeGetNextChar (tape);
      tapeMoveHead (tape, -1);
      goto L1;
      }
    }

  if (len) {
    *len = i;
    }

  return 1;
  }

/* NonBlankEntry = NoWSChar (RawChar* NoWSChar)?        # Not bracketted by WS
                 | ["] QuotableText ["] */
static int parseNonBlankEntry (int* len, struct tape* tape) {
  int c, j;

  c = tapeGetNextChar (tape);

  if (0 > c) {
    return 0;
    }

  /* ["] QuotableText ["] */
  if ('"' == c) {
    if (!parseQuotableText (&j, tape)) {
      tapeMoveHead (tape, -1);
      return 0;
      }

    c = tapeGetNextChar (tape);

    if ('"' != c) {
      tapeMoveHead (tape, - (2 + j));
      return 0;
      }

    if (len) {
      *len = 2 + j;  /* 2*["] + QuotableText */
      }

    return 1;
    }

  /* NoWSChar */
  if (!parse_isNoWSChar[c]) {
    tapeMoveHead (tape, -1);
    return 0;
    }

  /* (RawChar* NoWSChar)? */
  j = 0;
  c = tapeScanLen (tape, &j, parse_isRawChar);

  /* backup, in case RawChars overran a NoWSChar */
  while (0 <= c && !parse_isNoWSChar[c] && j > 0) {
    tapeMoveHead (tape, -1);
    j--;
    c = tape->b->data[tape->ofs];
    }

  if (0 > c || !parse_isNoWSChar[c]) {
    tapeMoveHead (tape, -j);

    if (len) {
      *len = 1;  /* NoWSChar alone */
      }

    return 1;
    }

  tapeMoveHead (tape, 1);

  if (len) {
    *len = 2 + j;  /* 2 * NoWSChar + the RawChar* sequence */
    }

  return 1;
  }

/* Convert doubled up quotes back to single quotes.  This function assumes that the
   bstring passed to it is modifiable and has a quote character at the beginning
   and the end. */
static void csvInPlaceFixQuotes (bstring b) {
  int i, d, len;
  len = b->slen - 1;

  for (d = 0, i = 1; i < len; i++, d++) {
    b->data[d] = b->data[i];

    if ('"' == b->data[i] && '"' == b->data[i + 1]) {
      i++;
      }
    }

  b->slen = d;
  }

enum parseFieldState {
  PFS_BEGINNING,
  PFS_NEXT_ENTRY
};

struct CSVStream {
  struct tape tape;
  struct tagbstring outsHdr;
  enum parseFieldState state;
  unsigned char* patchChar;
  unsigned char patchValue;
  };

/*
 *  Create a struct CSVStream on top of an existing struct bStream.
 */

struct CSVStream* parseCSVOpen (struct bStream* s) {
  struct CSVStream* csvs;

  if (!s) {
    return NULL;
    }

  initCSVParseCharsets ();

  if (NULL != (csvs = (struct CSVStream*) malloc (sizeof (struct CSVStream)))) {
    csvs->tape.s = s;
    csvs->tape.b = bfromcstr ("");
    csvs->tape.ofs = 0;
    csvs->state = PFS_BEGINNING;
    csvs->patchChar = NULL;
    }

  return csvs;
  }

/*
 *  Close the CSVStream.
 */

struct bStream* parseCSVClose (struct CSVStream* csvs) {
  struct bStream* s;

  if (!csvs) {
    return NULL;
    }

  s = csvs->tape.s;
  bdestroy (csvs->tape.b);
  csvs->tape.b = NULL;
  csvs->tape.ofs = -1;
  csvs->tape.s = NULL;
  return s;
  }

/*

   How the CSV data is incrementally parsed, from a high level point of view:

   Entry      = []
              | NonBlankEntry

   Field      = WSChar*                              # 0 entries
              | WSChar* NonBlankEntry WSChar*        # 1 entry
              | WSChar* Entry WSChar* [,] Field      # 2 or more entries

   CSVFile    = Field
              | Field [\r] [\n]? CSVFile

   Entry Parsing State Machine:

    [BEGIN]     [\r\n] -> Emit(CNM_NEXT_FIELD)                    -> [BEGIN]
                   EOF -> Emit(CNM_NEXT_END)                      -> Done
                     , -> consume character; Emit(CNM_NEXT_ENTRY) -> [NEXT ENTRY]
              else -> Emit(CNM_NEXT_ENTRY)                    -> [NEXT ENTRY]

  [NEXT ENTRY]     , -> consume character; Emit(CNM_NEXT_ENTRY) -> [NEXT ENTRY]
                else -> Emit(CNM_NEXT_FIELD)                    -> [BEGIN]
*/

/*
 *  Obtain the next entry from a CSVStream.  entry must point to a valid,
 *  writable struct CSVEntry which will be filled in as the result of this
 *  function if more CSV data is available.  If the end of the CSVStream has
 *  been reached 0 is returned, otherwise 1 is returned.
 */

int parseCSVNextEntry (struct CSVEntry* entry, struct CSVStream* csvs) {
  static struct tagbstring empty = bsStatic ("");
  int c, i, j, pos, len, ret;

  if (!entry || !csvs || !csvs->tape.s || !csvs->tape.b || csvs->tape.ofs < 0) {
    return -__LINE__;
    }

  /* undo a forced '\0' if necessary. */
  if (csvs->patchChar) {
    *csvs->patchChar = csvs->patchValue;
    csvs->patchChar = NULL; /* Turn off patch */
    }

  i = 0;
  c = tapeScanLen (&csvs->tape, &i, parse_isWSChar);

  if (0 > c) { /* Have we reached the end of the CSV file? */
    entry->mode = CNM_NEXT_END;
    entry->contents = NULL;
    return 0;
    }

  /* Discard any old/useless tape information. */
  if (0 > (ret = tapeSquashPrevious (&csvs->tape))) {
    return ret;
    }

  i = 0;

  if (PFS_BEGINNING == csvs->state) { /* First entry of the field? */
    if ('\r' == c) { /* Just a blank line, mean just advance the field counter. */
      tapeMoveHead (&csvs->tape, 1);
      c = tapeGetNextChar (&csvs->tape);

      if ('\n' != c) {
        tapeMoveHead (&csvs->tape, -1);
        }

      entry->mode = CNM_NEXT_FIELD;
      entry->contents = NULL;
      return 1;
      }

    else if ('\n' == c) {   /* Just a blank line, mean just advance the field counter. */
      tapeMoveHead (&csvs->tape, 1);
      entry->mode = CNM_NEXT_FIELD;
      entry->contents = NULL;
      return 1;
      }

    csvs->state = PFS_NEXT_ENTRY;

    if (',' == c) { /* An immediate comma means the first entry is blank */
      entry->mode = CNM_NEXT_ENTRY;
      entry->contents = &empty;
      return 1;
      }
    }

  else if (PFS_NEXT_ENTRY == csvs->state) {   /* After the first entry of the field? */
    if (',' != c) {
      return -__LINE__;
      }

    tapeMoveHead (&csvs->tape, 1);
    c = tapeScanLen (&csvs->tape, &i, parse_isWSChar);

    if ('\r' == c) { /* Trailing comma? */
      tapeMoveHead (&csvs->tape, 1);
      csvs->state = PFS_BEGINNING;
      c = tapeGetNextChar (&csvs->tape);

      if ('\n' != c) {
        tapeMoveHead (&csvs->tape, -1);
        }

      entry->mode = CNM_NEXT_FIELD;
      entry->contents = &empty;
      return 1;
      }

    else if ('\n' == c) {   /* Trailing comma? */
      tapeMoveHead (&csvs->tape, 1);
      csvs->state = PFS_BEGINNING;
      entry->mode = CNM_NEXT_FIELD;
      entry->contents = &empty;
      return 1;
      }

    if (0 > c) { /* End */
      entry->mode = CNM_NEXT_END;
      entry->contents = &empty;
      return 0;
      }

    if (',' == c) { /* empty entry */
      entry->mode = CNM_NEXT_ENTRY;
      entry->contents = &empty;
      return 1;
      }
    }

  else {
    return -__LINE__;
    }

  /* We know we must have a non-blank entry here. */
  j = 0;

  if (0 == parseNonBlankEntry (&j, &csvs->tape)) {
    return -__LINE__;
    }

  pos = csvs->tape.ofs - j;
  len = j;

  /* Consume trailing white space after entry. */
  i += j;
  c = tapeScanLen (&csvs->tape, &i, parse_isWSChar);

  /* Build reference to this parsed entry directly from the tape. */
  bmid2tbstr (csvs->outsHdr, csvs->tape.b, pos, len);

  /* Convert doubled quotes back into singled quotes if necessary. */
  if ('"' == csvs->outsHdr.data[0]) {
    csvInPlaceFixQuotes (&csvs->outsHdr);
    }

  /* Force terminating '\0' so that it can be directly output via printf if
     so desired.  We know there is space here since the bsread operations
     also always add a terminating '\0' on the end as well. */
  csvs->patchChar = csvs->outsHdr.data + csvs->outsHdr.slen;
  csvs->patchValue = *csvs->patchChar;
  *csvs->patchChar = (unsigned char) '\0';
  entry->contents = &csvs->outsHdr;

  if (0 > c) {
    entry->mode = CNM_NEXT_END;
    return 0;
    }

  entry->mode = CNM_NEXT_ENTRY;

  /* Deal with the case of this being the last entry of the field. */
  if ('\r' == c) {
    tapeMoveHead (&csvs->tape, 1);
    csvs->state = PFS_BEGINNING;
    c = tapeGetNextChar (&csvs->tape);

    if ('\n' != c) {
      tapeMoveHead (&csvs->tape, -1);
      }

    entry->mode = CNM_NEXT_FIELD;
    }

  else if ('\n' == c) {
    tapeMoveHead (&csvs->tape, 1);
    csvs->state = PFS_BEGINNING;
    entry->mode = CNM_NEXT_FIELD;
    }

  return 1;
  }

/*
 *  Create an output stream for CSV entries.  The CSVWriteStream options
 *  msdos and alwaysQuote flags can be modified after the fact.
 */

int csvInitWriteStream (struct CSVWriteStream* os, size_t (* fw) (const void* buf, size_t elsize, size_t nelem, void* parm), void* parm) {
  if (NULL == fw) {
    return -__LINE__;
    }

  os->fw = fw;
  os->parm = parm;
  os->prevMode = CNM_NEXT_FIELD;
  os->msdos = 1;
  os->alwaysQuote = 0;
  return 0;
  }

/*
 *  Append an entry to the end of the CSV output stream.  The caller must
 *  fill entry with the right mode (one of CNM_NEXT_ENTRY, CNM_NEXT_FIELD,
 *  or CNM_NEXT_END) and the raw contents as a bstring.  The string will
 *  be quoted automatically.
 */

int csvAppend (struct CSVWriteStream* os, const struct CSVEntry* entry) {
  static struct tagbstring needquotes = bsStatic ("\",\r\n");
  int ret;
  bstring b;

  if (!os || !entry) {
    return -__LINE__;
    }

  if (CNM_NEXT_END == os->prevMode) {
    return -__LINE__;
    }

  if (entry->mode == CNM_NEXT_END) {
    os->prevMode = entry->mode;

    if (NULL != entry->contents &&
        0 != entry->contents->slen &&
        1 != os->fw (entry->contents->data, entry->contents->slen, 1, os->parm)) {
      return -__LINE__;
      }

    return 0;
    }

  if (NULL == entry->contents) {
    if (CNM_NEXT_FIELD != entry->mode) {
      return -__LINE__;
      }

    os->prevMode = entry->mode;

    if (os->msdos) {
      return 1 == os->fw (bsStaticBlkParms ("\r\n"), 1, os->parm);
      }

    else {
      return 1 == os->fw (bsStaticBlkParms ("\r"), 1, os->parm);
      }
    }

  /* Special case a field with a single empty entry in it. */
  if (CNM_NEXT_FIELD == entry->mode &&
      0 == blength (entry->contents) &&
      CNM_NEXT_FIELD == os->prevMode) {
    if (os->msdos) {
      return 1 == os->fw (bsStaticBlkParms ("\"\"\r\n"), 1, os->parm);
      }

    else {
      return 1 == os->fw (bsStaticBlkParms ("\"\"\r"), 1, os->parm);
      }
    }

  if ( (entry->contents->slen > 0 && (parse_isWSChar[entry->contents->data[0]] || parse_isWSChar[entry->contents->data[entry->contents->slen - 1]])) || BSTR_ERR != binchr (entry->contents, 0, &needquotes)) {
    int i, len = blength (entry->contents);
    b = bfromcstr ("\"");

    for (i = 0; i < len; i++) {
      if ('"' == entry->contents->data[i]) {
        bconchar (b, (char) '"');
        }

      bconchar (b, entry->contents->data[i]);
      }

    bconchar (b, (char) '"');
    ret = (int) os->fw (b->data, b->slen, 1, os->parm);
    bdestroy (b);

    if (1 != ret) {
      return -__LINE__;
      }
    }

  else {
    if (os->alwaysQuote) {
      if (0 == entry->contents->slen) {
        if (1 != os->fw (bsStaticBlkParms ("\"\""), 1, os->parm)) {
          return -__LINE__;
          }
        }

      else {
        if (1 != os->fw (bsStaticBlkParms ("\""), 1, os->parm) ||
            1 != os->fw (entry->contents->data, entry->contents->slen, 1, os->parm) ||
            1 != os->fw (bsStaticBlkParms ("\""), 1, os->parm)) {
          return -__LINE__;
          }
        }
      }

    else {
      if (0 != entry->contents->slen &&
          1 != os->fw (entry->contents->data, entry->contents->slen, 1, os->parm)) {
        return -__LINE__;
        }
      }
    }

  os->prevMode = entry->mode;

  if (CNM_NEXT_FIELD == entry->mode) {
    if (os->msdos) {
      return 1 == os->fw (bsStaticBlkParms ("\r\n"), 1, os->parm);
      }

    else {
      return 1 == os->fw (bsStaticBlkParms ("\r"), 1, os->parm);
      }
    }

  return 1 == os->fw (bsStaticBlkParms (","), 1, os->parm);
  }

#ifdef MAIN_TEST_FOR_CSV

int benchmarkMode = 0;

int main (int argc, char* argv[]) {
  FILE* fp;
  struct bStream* s;
  int ret;
  int rept = 1;
  clock_t start, end;
  char* outputFile = NULL;

  if (argc >= 2 && argv[1][0] == '-') {
    if (argv[1][1] == 'o') {
      outputFile = argv[1] + 2;
      argv++;
      argc--;
      }
    }

  if (argc < 2) {
    fprintf (stderr, "%s [-oreformatted-output] csv-file [repeat-count]\n", argv[0]);
    return -__LINE__;
    }

  if (argc >= 3) {
    sscanf (argv[2], "%d", &rept);

    if (rept < 1) {
      rept = 1;
      }

    benchmarkMode = 1;
    }

  start = clock();
L1:
  ;

  if (NULL == (fp = fopen (argv[1], "rb"))) {
    fprintf (stderr, "Unable to read %s\n", argv[1]);
    return -__LINE__;
    }

  if (NULL == (s = bsopen ( (bNread) fread, fp))) {
    fclose (fp);
    fprintf (stderr, "Unable to allocate stream [%d]\n", __LINE__);
    return -__LINE__;
    }

    {
    struct CSVStream* csvs = parseCSVOpen (s);
    struct CSVEntry entry;

    if (csvs) {
      struct {
        int row, col;
        } coords = { 0, 0 };
      FILE* ofp;
      struct CSVWriteStream os;

      ofp = NULL;

      if (!benchmarkMode) {
        if (outputFile) {
          ofp = fopen (outputFile, "wb");

          if (NULL != ofp) {
            csvInitWriteStream (&os, (size_t (*) (const void * buf, size_t elsize, size_t nelem, void * parm)) fwrite, ofp);
            }
          }
        }

      while (0 <= (ret = parseCSVNextEntry (&entry, csvs))) {
        if (!benchmarkMode) {
          if (NULL != ofp) {
            csvAppend (&os, &entry);
            }

          if (entry.contents) {
            printf ("(%d,%d):%s\n", coords.row, coords.col, (const char*) entry.contents->data);
            }
          }

        switch (entry.mode) {
          case CNM_NEXT_ENTRY:
            coords.col++;
            break;

          case CNM_NEXT_FIELD:
            coords.col = 0;
            coords.row++;
            break;

          case CNM_NEXT_END:

            if (!benchmarkMode) {
              printf ("<EOF>\n");
              }

            break;

          default:
            return -__LINE__;
          }

        if (0 == ret) {
          break;
          }
        }

      if (0 > ret) {
        fprintf (stderr, "Error in CSV parsing\n");
        }

      parseCSVClose (csvs);

      if (NULL != ofp) {
        fclose (ofp);
        }
      }
    }

  bsclose (s);
  fclose (fp);

  rept--;

  if (rept > 0) {
    goto L1;
    }

  end = clock();

  if (benchmarkMode) {
    fprintf (stderr, "Time: %fs\n", (end - start) / (double) CLOCKS_PER_SEC);
    }

  return 0;
  }

#endif
