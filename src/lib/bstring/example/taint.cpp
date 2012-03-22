/*
 *  This is a demonstration of using the standard C++ type system to
 *  create a CUntaintedString class which operates similarly to a
 *  CBString, while presenting a shell() interface and allowing only
 *  explicit promotion of strings that are not "Untainted".  This provides
 *  similar functionality to Perl's "tainted/untainted" attribute while
 *  being able to enforce it at compile time rather than relying on late
 *  on-the-fly detection.  (That is to say, instrinsic coding problems
 *  should be presented to developers to be fixed, not to end users.)
 */

#include <stdio.h>
#include <stdlib.h>

#define BSTRLIB_DONT_ASSUME_NAMESPACE
#include "bstrwrap.h"

#if defined(__WATCOMC__)
#define POPEN_SUPPORTED
#define popen(x,y) _popen(x,y)
#endif

class CUntaintedString {
    // This class must not have any friends.

  private:
    Bstrlib::CBString s;

  public:
    // Constructors.  The constructors cannot automatically promote
    // from another string type, otherwise the compiler would be
    // compelled to use them in situations where we do not wish them
    // to be allowed.
    CUntaintedString () {
      s = "";
      }

    // Destructor
    ~CUntaintedString () {}

    // This is the only way to fill the contents of a CUntaintedString
    // from another type of string.  The point is that auditing is
    // reduced to grepping the code for calls to "system" and "promote".
    // Its also possible to do runtime string authentication by parsing
    // the string for legality at this point.
    CUntaintedString& promote (const Bstrlib::CBString& b) {
      s = b;
      return *this;
      }

    // Send to shell (the whole point of this)
    int shell (void) const;
#ifdef POPEN_SUPPORTED
    FILE* shellstdout (const char* mode) const;
#endif

    // Assignment and add operators.
    const CUntaintedString& operator  = (const CUntaintedString&);
    const CUntaintedString& operator += (const CUntaintedString&);
    const CUntaintedString  operator  + (const CUntaintedString&) const;

    // Standard comparison operators.
    inline bool operator == (const Bstrlib::CBString& b) const {
      return s == b;
      }
    inline bool operator != (const Bstrlib::CBString& b) const {
      return s != b;
      }
    inline bool operator < (const Bstrlib::CBString& b) const {
      return s <  b;
      }
    inline bool operator <= (const Bstrlib::CBString& b) const {
      return s <= b;
      }
    inline bool operator > (const Bstrlib::CBString& b) const {
      return s >  b;
      }
    inline bool operator >= (const Bstrlib::CBString& b) const {
      return s >= b;
      }

    // Cast downward to const CBString.
    inline operator const Bstrlib::CBString () const {
      return s;
      }

    // Accessors
    inline int length () const {
      return this->s.length ();
      }
    inline unsigned char character (int i) const {
      return this->s.character (i);
      }
    inline unsigned char operator [] (int i) const {
      return character (i);
      }
  };

int CUntaintedString::shell (void) const {
  return system ( (const char*) (const Bstrlib::CBString) * this);
  }

#ifdef POPEN_SUPPORTED
FILE* CUntaintedString::shellstdout (const char* mode) const {
  return popen ( (const char*) (const Bstrlib::CBString) * this, mode);
  }
#endif

int isShellAvailable (void) {
  return system (NULL);
  }

const CUntaintedString& CUntaintedString::operator = (const CUntaintedString& b) {
  this->s = b.s;
  return *this;
  }

const CUntaintedString& CUntaintedString::operator += (const CUntaintedString& b) {
  this->s += b.s;
  return *this;
  }

const CUntaintedString CUntaintedString::operator + (const CUntaintedString& b) const {
  CUntaintedString ret;
  ret.promote (this->s + b.s);
  return ret;
  }

int main () {
  CUntaintedString t, u;
  Bstrlib::CBString v;

  // We decide that the following are safe:
  t.promote ("echo ");
  u.promote ("test");

  // The == operator works because t is automatically cast to a
  // CBString.
  if (t == "echo ") {
    puts ("The echo command is about to be executed.");
    }

  // This cast down must be done in two steps.  This removes certain
  // ambiguities.
  printf ("To be executed: shell(\"%s\")\n", (const char*) (const Bstrlib::CBString) (t + u));

  v = Bstrlib::CBString ("echo exploit; ") + Bstrlib::CBString (u);
  printf ("Not executed:   shell(\"%s\")\n", (const char*) v);

  // The + operator works on two untainted strings to produce an
  // untainted string.
  if (isShellAvailable ()) {
    (t + u).shell ();
    }

#ifdef POPEN_SUPPORTED
  FILE* fp = (t + u).shellstdout ("r");

  if (fp) {
    Bstrlib::CBString c ("");
    c.gets ( (bNgetc) fgetc, fp);
    fclose (fp);
    printf (">> %s\n", (const char*) c);
    }

#endif

  // These guys are all illegal and detected at *COMPILE TIME*, because
  // they have not been processed through "promote".
  //
  // (CUntaintedString ("echo exploit")).shell ();
  // (t + "test ; echo exploit").shell ();
  // (t + Bstrlib::CBString ("test ; echo exploit")).shell ();
  // t[0] = ';';

  return 0;
  }
