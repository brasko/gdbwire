#include "config.h"
#include "fixture.h"

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

std::string
Fixture::src()
{
    std::string srcdir = GDBWIRE_ABS_TOP_SRCDIR;
    srcdir += "/src";
    srcdir += "/progs";
    srcdir += "/test_suite";
    return srcdir;
}

std::string
Fixture::data()
{
    std::string datadir = src();
    datadir += "/data";
    return datadir;
}

std::string
Fixture::sourceTestDir()
{
    std::string datadir = data();
    datadir = datadir + "/" + testCaseName();
    datadir = datadir + "/" + testName();
    return datadir;
}

std::string
Fixture::dest()
{
    std::string builddir = GDBWIRE_ABS_TOP_BUILDDIR;
    builddir += "/results";
    return builddir;
}

std::string
Fixture::destTestDir()
{
    std::string destdir = dest();
    destdir = destdir + "/" + testCaseName();
    destdir = destdir + "/" + testName();
    return destdir;
}

void
Fixture::copy(const std::string &src, const std::string &dest, bool recursive)
{
#if 0
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
#endif
}

void
Fixture::copyTestData()
{
#if 0
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
#endif
}
