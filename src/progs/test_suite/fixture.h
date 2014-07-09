#ifndef __FIXTURE_H__
#define __FIXTURE_H__

#include <string>

/**
 * Provides functionality above and beyond the normal Test.
 *
 * This test fixture provides a simple framework for dealing with
 * the file system while performing a unit test.
 *
 * It provides a standard location to read test data from the src tree.
 *
 * Any test inheriting from this test fixture will automatically
 * know where to get any necessary related data files required for testing.
 *
 * The source directory for the test case comes from,
 *   $abs_top_srcdir/progs/test_suite/data/TestName
 * Please note that TestName is a hierarchical name. See testName()
 * for more information on this.
 */
class Fixture {
    public:

        virtual ~Fixture() {}

        /**
         * Get the name of the test currently running.
         *
         * The name of the test is hierarchical. The format is
         * root/node1/node2/.../leaf. An example would be,
         *   GdbmiTest/basic
         * or
         *   GdbwireStringTest/append_cstr/null_value 
         *   GdbwireStringTest/append_cstr/null_instance 
         *
         * When using the macro TEST_CASE_METHOD_N below please note that
         * the fixture name is always the first piece of the test name. So
         * in the examples above that means GdbmiTest or GdbwireStringTest.
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
         * Get the test specific path (file or directory) in the source tree.
         *
         * For example
         *   $abs_top_srcdir/progs/test_suite/data/TestName.
         * where TestName could refer to a file or directory.
         *
         * @return
         * The absolute path to the test specific path (file or directory)
         * contained in the source tree this test suite is running against.
         * This may be read-only.
         */
        std::string sourceTestPath();
};

// A convience macro for creating unit tests.
#define TEST_CASE_METHOD_N(Fixture, name) \
    TEST_CASE_METHOD(Fixture, #Fixture "/" #name)

#endif /* __FIXTURE_H__ */
