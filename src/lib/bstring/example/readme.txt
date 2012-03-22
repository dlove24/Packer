This package contains examples of how to use the Better String Library.  Each
example is a single file which should be compiled and linked with bstrlib.
For example:

    gcc -O2 -Ibstrlib/ strcat.c ./bstrlib/bstrlib.c

assuming you have a subdirectory called bstrlib which contains the bstrlib
library sources.

Makefiles have also been provided for some popular compilers as well.  A
subdirectory named "obj" may need to be created prior to using some of the
makefiles.

The re.c module is an example demonstrating that interoperating with the PCRE 
(Perl Compatible Regular Expressions) library is straightforward.  It requires
linking against bstrlib and the pcre library (for the Windows DLL version
remove the #define PCRE_STATIC at the start of re.c).  For more information 
on using and obtaining PCRE see http://www.pcre.org

The subdirectory mfcbench contains a benchmark for comparing to Microsoft
Foundation Classes.  It is only known to build correctly for Microsoft Visual
C++ and Intel C++.

