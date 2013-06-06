#include "config.h"
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include "fixture.h"

using namespace boost::filesystem;

Fixture::Fixture()
{
    copyTestData();
}

Fixture::~Fixture()
{
}

std::string
Fixture::testCaseName()
{
    const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
    return test_info->test_case_name();
}

std::string
Fixture::testName()
{
    const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();
    return test_info->name();
}

path
Fixture::src()
{
    path srcdir = GDBWIRE_ABS_TOP_SRCDIR;
    srcdir /= "progs";
    srcdir /= "test_suite";
    return srcdir;
}

path
Fixture::data()
{
    path datadir = src();
    datadir /= "data";
    return datadir;
}

path
Fixture::sourceTestDir()
{
    path datadir = data();
    datadir /= testCaseName();
    datadir /= testName();
    return datadir;
}

path
Fixture::dest()
{
    path builddir = GDBWIRE_ABS_TOP_BUILDDIR;
    builddir /= "results";
    return builddir;
}

path
Fixture::destTestDir()
{
    path destdir = dest();
    destdir /= testCaseName();
    destdir /= testName();
    return destdir;
}

void
Fixture::copy(const path &src, const path &dest, bool recursive)
{
    if (!exists(src)) {
        // throw file not found exception src
    } else if (!exists(dest.parent_path())) {
        // throw file not found exception for dest parent path
    } else if (is_directory(src) && !recursive) {
        // throw exception if trying to copy a directory and not recursive
    }

    // Determine the target path name
    path target = dest;
    if (is_directory(dest)) {
        target /= src.filename();
    }

    // Do the copy (separate into recursive and non-recursive copy)
    if (recursive && is_directory(src)) {
        // Does this throw an exception on failure?
        copy_directory(src, target);
        for (directory_iterator end, iter(src);  iter != end; ++iter) {
            copy(iter->path(), target, recursive);
        }
    } else {
        copy(src, target);
    }
}

void
Fixture::copyTestData()
{
    path sourceTestDirectory = sourceTestDir();
    path destTestDirectory = destTestDir();
    path destTestCaseDirectory(destTestDirectory.parent_path());

    // Delete the test directory if it exists, to start fresh
    if (exists(destTestDirectory)) {
        remove_all(destTestDirectory.filename());
    }
    
    // The Test Case directory may not exist if this is
    // the first test to run in this Test Case, create it then.
    if (!exists(destTestCaseDirectory)) {
        //VASSERT(QDir().mkpath(destTestCaseDirectory.path()));
    }

    // Finally, if the Test directory exists in the source tree then
    // copy it to the destination tree. Otherwise do nothing as the test
    // must not require files.
    if (exists(sourceTestDirectory)) {
        copy(sourceTestDirectory, destTestCaseDirectory);
    }
}
