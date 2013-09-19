#ifndef __FIXTURE_H__
#define __FIXTURE_H__

#include <string>

/**
 * Provides functionality above and beyond the normal Test.
 *
 * This test fixture provides a simple framework for dealing with
 * the file system while performing a unit test.
 *
 * It provides a standard location to read test data and a
 * standard location to write test data. It also provides the
 * ability to automatically copy test data from the source tree
 * to the test working directory.
 *
 * Any test inheriting from this test fixture will automatically
 * perform the copy of the test data from the source directory
 * to the destination directory in the constructor of this class.
 *
 * The source directory for the test case comes from,
 *   $abs_top_srcdir/progs/test_suite/data/TestName
 * The destination directory for the test case will be,
 *   $abs_top_builddir/test_suite/TestName
 *
 * Please note that TestName is a hierarchical name. See testName()
 * for more information on this.
 */
class Fixture {
    public:

        Fixture();
        virtual ~Fixture();

        /**
         * Get the name of the test currently running.
         *
         * The name of the test is hierarchical. The format is
         * root/node1/node2/.../leaf. The slash separator is convient as
         * it allows you to split up the test case into a hierarchy and
         * it also can be used as a unique directory name.
         *
         * An example would be,
         *   GdbmiTest/basic
         * or
         *   append_cstr/null_value 
         *   append_cstr/null_instance 
         *
         * @return
         * The test name of the test currently running.
         */
        std::string testName();

        /**
         * Get the source directory this test suite is running against.
         *
         * For example $abs_top_srcdir/progs/test_suite.
         *
         * @return
         * The absolute path to the source directory this test suite is
         * running against. This may be read-only.
         */
        std::string src();

        /**
         * Get the data directory in the source tree containing test files.
         *
         * For example $abs_top_srcdir/progs/test_suite/data.
         *
         * @return
         * The absolute path to the data directory contained in the source
         * directory this test suite is running against. This may be read-only.
         */
        std::string data();

        /** 
         * Get the test specific directory in the source directory.
         *
         * For example
         *   $abs_top_srcdir/progs/test_suite/data/TestCaseName/TestName.
         *
         * @return
         * The absolute path to the test specific directory contained in the
         * source directory this test suite is running against.
         * This may be read-only.
         */
        std::string sourceTestDir();
        
        /**
         * Get the destination directory this test suite is running in.
         *
         * This is useful to allow tests to create temporary files and
         * folders as needed to complete their tests.
         *
         * For example $abs_top_builddir/results.
         *
         * @return
         * The absolute path to the destination directory this test suite is
         * running against. This is intended to be writable.
         */
        std::string dest();

        /** 
         * Get the test specific directory in the destination directory.
         *
         * For example $abs_top_builddir/results/TestCaseName/TestName.
         *
         * @return
         * The absolute path to the test specific directory contained in the
         * destination directory this test suite is running against.
         * This is intended to be writable.
         */
        std::string destTestDir();


        /**
         * Copy the source directory to the destination directory.
         *
         * This is a non recursive directory copy. It only copies a
         * single source directory and all the files in it to the
         * destionation directory.
         *
         * The destination directory may exist. If it doesn't, it
         * will be created. The destination directory may not contain
         * a folder in it that is the basename of the source directory.
         * This function is required to make and create that directory.
         *
         * @param src
         * The src directory to copy, and all the files in it.
         *
         * @param dest
         * The destination directory.
         */
        void copy(const std::string &src, const std::string &dest,
                bool recursive = false);

    private:
        /**
         * Copy the test data from the src dir to the destination dir.
         *
         * Each test case may have some tests that contain data on disk
         * necessary to run the test.
         *
         * This function copies the data from the src tree into the
         * destination tree and sets the destination test directory
         * member variable.
         */
        void copyTestData();
};

// A convience macro for creating unit tests.
#define TEST_F(Fixture, name) TEST_CASE_METHOD(Fixture, #Fixture "/" #name)

#endif /* __FIXTURE_H__ */
