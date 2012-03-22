#ifndef CSV_PARSE_MODULE_INCLUDED
#define CSV_PARSE_MODULE_INCLUDED

#include "bstrlib.h"

#ifdef __cplusplus
extern "C" {
#endif

  enum csvNextMode {
    CNM_NEXT_ENTRY,
    CNM_NEXT_FIELD,
    CNM_NEXT_END
    };

  struct CSVEntry {
    struct tagbstring* contents;
    enum csvNextMode mode;
    };

  struct CSVStream;

  extern struct CSVStream* parseCSVOpen (struct bStream* s);
  extern struct bStream* parseCSVClose (struct CSVStream* csvs);
  extern int parseCSVNextEntry (struct CSVEntry* entry, struct CSVStream* csvs);

  struct CSVWriteStream {
    size_t (* fw) (const void* buf, size_t elsize, size_t nelem, void* parm);
    void* parm;
    enum csvNextMode prevMode;
    int msdos: 1;
    int alwaysQuote: 1;
    };

  extern int csvAppend (struct CSVWriteStream* os, const struct CSVEntry* entry);
  extern int csvInitWriteStream (struct CSVWriteStream* os, size_t (* fw) (const void* buf, size_t elsize, size_t nelem, void* parm), void* parm);

#ifdef __cplusplus
  }
#endif

#endif
