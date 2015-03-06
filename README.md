# Welcome To GDBWIRE

GDBWIRE is a portable lightweight library for communicating with the [GNU debugger] (http://www.gnu.org/software/gdb/).

It is intended to be used by developers that would like to communicate with
GDB using the [the GDB/MI interface](https://sourceware.org/gdb/onlinedocs/gdb/GDB_002fMI.html), but with out reinventing the wheel.

GDBWIRE communicates with GDB using the GDB/MI interface. It has several
core features that make it unique.

- Portable - Written in C and runs on Linux, Windows (mingw or cygwin) and Mac
- Efficient - Written using Bison and Flex
- Asynchronous - Uses a Bison push parser which will not block your main loop
- Reentrant - Supports the use of threads in your application code
- Tested - Has a robust test suite with 100% parser code coverage
- Low Level - You provide GDB output characters, GDBWIRE provides parse trees
- High Level - TODO: You provide GDB output characters, GDBWIRE provides events
- Error Handling - Unexpected GDB output will be handled gracefully

## Examples

GDBWIRE currently provides an example of the low level interface described
above in the [gdbmi example](src/progs/examples/gdbmi_example.c) program.

This program is particularly useful when run from the command line, from the
build directory, as follows,
>   gdb -i=mi <gdb arguments> | examples/gdbmi

This allows a user to interact with gdb in arbitrary ways from the 
command line. At the same time, the user can determine if the gdbmi
parser is able to handle the gdbmi output created by gdb. If the GDBWIRE
parser fails, it will output the GDB/MI line that it could not handle.

## Build Instructions

### Dependencies

You must have the following packages installed.
- sh
- autoconf
- automake
- aclocal
- autoheader
- flex
- bison
- gcc/g++
- libtool

Under Ubuntu, you can install all the dependencies using the following command:

```
sudo apt-get install autoconf automake flex bison build-essential libtool
```

### Preparing the configure

Run ./autogen.sh in the current working directory to generate the configure
script.

### Running configure, make and make install

You can run ./configure from within the source tree, however I usually run
configure from outside the source tree like so:

```
>  mkdir ../build
>  cd ../build
>  YFLAGS="-Wno-deprecated" CFLAGS="-g -Wall -Werror" CXXFLAGS="-g -Wall -Werror" ../gdbwire/configure --prefix=$PWD/../prefix --enable-tests --enable-examples
>  make -srj4
>  make install
``

The make install rule is optional. You can simply run make and the executables
will be in there respective build directories. 

If you like to have a silent build, and the libtool link lines are bothering
you, you can set this environment variable to suppress libtools printing of
the link line,
>  LIBTOOLFLAGS=--silent

## Running the test suite

If you want to run the test suite you have to tell the build system
to build it. This is done by passing --enable-tests on the configure line.

After you have built the test suite, you can run all the unit tests with
the command,
>  ./test\_suite

Ensure that you introduce no memory leaks or memory errors in the test suite.
You can do this with the command,
>  LD_LIBRARY_PATH=$PWD/.libs/ valgrind  --leak-check=full --tool=memcheck ./.libs/test_suite

in the directory where the test\_suite executable is created. You should
expect to see valgrind output something like,
 
    HEAP SUMMARY:
        in use at exit: 0 bytes in 0 blocks
        total heap usage: X allocs, X frees, Y bytes allocated

        All heap blocks were freed -- no leaks are possible

        For counts of detected and suppressed errors, rerun with: -v
        ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

## An overview of the source code

directory               | description
---                     | ---
src                     | All source code is in here
src/progs               | All programs go here
src/progs/test\_suite   | The unit test executable
src/progs/examples      | Example programs using the gdbwire interfaces
src/lib                 | All libraries go here
src/lib/containers      | All containers go here (string, list, etc)
src/lib/logging         | Error handling and potential logging of errors
src/lib/gdbmi           | The gdbmi interface

## Developer Conventions

Proper Error handling is an important part of gdbwire. This library
is intended to be used by other applications and it is important to provide
appropriate error messages to the end users when necessary.

Result codes have been chosen as the primary means of returning status
from a function. The result codes are an enumeration called gdbwire_result,
which is defined in the file gdbwire_result.h. GDBWIRE_OK is a successful
result status and all others indicate a particular error condition.

Not all functions need to return a result code. A function may return
void if it can not fail. A function can return a NULL pointer when it makes
sense to do so. Common sense should be applied.

As a rule of thumb, error conditions should be handled if possible.
Otherwise they should bubble up. This should repeat at each level until
the gdbwire user receives the proper error result.

It is encouraged that all function interfaces are documented and that
the preconditions of the functions are clearly stated. The function body
is encouraged to assert those preconditions to catch errors as soon as
possible. Some macros have been added to help with this matter.
GDBWIRE_ASSERT and GDBWIRE_ASSERT_ERRNO in gdbwire_assert.h are intended
to help the developer both assert, log and return an appropriate
error status on assertion failure. GDBWIRE_ASSERT_GOTO has been added
to avoid duplicating error handling code in a function. The common
error handling code can be written in a label that similar error
states can jump to. Other macros may be added to help facilitate
automatic error checking, logging and error propagation.

Finally, the gdbwire users will most likely hit issues in the field
that the source code in gdbwire does not expect. In these situations
it is appropriate to use the gdbwire_logger interface to log all
information that might be useful to help reproduce or fix an issue
that the user has encountered. The least amount of useful information
would be optimal.
