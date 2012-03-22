#include <stdio.h>
#include "bstrlib.h"

/*
 *  In the usenet newsgroup comp.lang.c Ramprasad A Padmanabhan wrote the
 *  following message:
 *
 *    In my program I am checking wether a emailid exists in a list
 *
 *    I have in the complete_list a string like
 *    "<aa@abc.com> <ab@abc.com> <bc@abc.com> ... <xy@abc.com>"  that is a
 *    string of emailids sorted
 *
 *    Now I want to find out if another ID  "<lm@abc.com>" exists in this list
 *
 *    How is it possible to optimize the search  given that the complete list
 *    string is sorted in order
 *
 *  Below is a demonstration of how this problem is solved in a very simple
 *  manner using the bsplitcb scanning function.
 */

struct cbsScanEmailAddress {
  bstring db, newID;
  int ret;
  };

static int testID (void* parm, int ofs, int len) {
  struct cbsScanEmailAddress* s = (struct cbsScanEmailAddress*) parm;
  struct tagbstring t;

  /* Wrap the substring corresponding to this entry by reference in t */
  blk2tbstr (t, s->db->data + ofs, len);
  s->ret = bstrcmp (&t, s->newID);

  /* A negative value indicates the split iteration should stop */
  return - (s->ret >= 0);
  }

static int scanForEmailAddress (const bstring dbIDs, const bstring newID) {
  struct cbsScanEmailAddress s;

  s.db = dbIDs;
  s.newID = newID;

  /* Iterate over the input string, seperating by the ' ' character */
  bsplitcb (dbIDs, (char) ' ', 0, testID, &s);
  return s.ret == 0;
  }

int main (int argc, char* argv[]) {
  struct tagbstring dbID = bsStatic ("<aa@abc.com> <ab@abc.com> " \
                                     "<bc@abc.com> <xy@abc.com>");
  struct tagbstring newID1 = bsStatic ("<bc@abc.com>");
  struct tagbstring newID2 = bsStatic ("<xbc@abc.com>");

  printf ("%d\n", scanForEmailAddress (&dbID, &newID1));
  printf ("%d\n", scanForEmailAddress (&dbID, &newID2));
  return 0;
  }

