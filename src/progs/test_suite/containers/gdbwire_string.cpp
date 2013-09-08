#include <gtest/gtest.h>

#include "fixture.h"
#include "containers/gdbwire_string.h"

namespace {
    struct GdbwireStringTest : public Fixture {
        void SetUp() {
            string = gdbwire_string_create();
            ASSERT_TRUE(string);
        }

        void TearDown() {
            gdbwire_string_destroy(string);
        }

        void validate(gdbwire_string *instance, size_t size,
                size_t capacity, const std::string &data) {
            std::string actual = std::string(gdbwire_string_data(instance),
                    gdbwire_string_size(instance));

            ASSERT_EQ(size, gdbwire_string_size(instance));
            ASSERT_EQ(capacity, gdbwire_string_capacity(instance));
            ASSERT_EQ(data, actual);
        }

        gdbwire_string *string;
    };
}

TEST_F(GdbwireStringTest, destroy_null_instance)
{
    gdbwire_string_destroy(NULL);
}

TEST_F(GdbwireStringTest, validateInitialState)
{
    validate(string, 0, 128, "");
}

TEST_F(GdbwireStringTest, append_cstr_null_value)
{
    ASSERT_EQ(-1, gdbwire_string_append_cstr(string, NULL));
}

TEST_F(GdbwireStringTest, append_cstr_null_instance)
{
    ASSERT_EQ(-1, gdbwire_string_append_cstr(NULL, "hi"));
}

TEST_F(GdbwireStringTest, append_cstr)
{
    // Append empty string to emptry string and check the state
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, ""));
    validate(string, 0, 128, "");

    // Append a character and check the state
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "a"));
    validate(string, 1, 128, "a");

    // Append empty string to non empty string and check the state
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, ""));
    validate(string, 1, 128, "a");

    // Append another a character and check the state
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "a"));
    validate(string, 2, 128, "aa");

    // Append a longer string and check the state
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "abc"));
    validate(string, 5, 128, "aaabc");

    // Append until size is 127 + a NULL character
    std::string longstring(122, 'd');
    std::string expected = std::string("aaabc") + longstring;

    ASSERT_EQ(0, gdbwire_string_append_cstr(string, longstring.c_str()));
    validate(string, 127, 128, expected);

    // Append just one more, to push up capacity
    expected += "e";

    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "e"));
    validate(string, 128, 256, expected);
}

TEST_F(GdbwireStringTest, append_data_null_value)
{
    ASSERT_EQ(-1, gdbwire_string_append_data(string, NULL, 0));
}

TEST_F(GdbwireStringTest, append_data_null_instance)
{
    ASSERT_EQ(-1, gdbwire_string_append_data(NULL, "a", 1));
}

TEST_F(GdbwireStringTest, appendData)
{
    std::string expected;

    // Append empty string to emptry string and check the state
    ASSERT_EQ(0, gdbwire_string_append_data(string, "", 0));
    validate(string, 0, 128, expected);

    // Append a character and check the state
    expected.append("a", 1);
    ASSERT_EQ(0, gdbwire_string_append_data(string, "a", 1));
    validate(string, 1, 128, expected);

    // Append empty string to a non emptry string and check the state
    ASSERT_EQ(0, gdbwire_string_append_data(string, "", 0));
    validate(string, 1, 128, expected);

    // Append another string with binary data  and check the state
    expected.append("a", 2);
    ASSERT_EQ(0, gdbwire_string_append_data(string, "a", 2));
    validate(string, 3, 128, expected);

    // Ensure the NULL character was written to the string
    ASSERT_EQ('\0', gdbwire_string_data(string)[2]);

    // Append another a character and check the state
    expected.append("ad", 2);
    ASSERT_EQ(0, gdbwire_string_append_data(string, "ad", 2));
    validate(string, 5, 128, expected);

    // Append until size is 128 with no trailing NULL character
    std::string longstring(123, 'd');
    expected += longstring;

    ASSERT_EQ(0, gdbwire_string_append_data(
            string, longstring.data(), longstring.size()));
    validate(string, 128, 128, expected);

    // Append just one more, to push up capacity
    expected += "e";

    ASSERT_EQ(0, gdbwire_string_append_data(string, "e", 1));
    validate(string, 129, 256, expected);
}

TEST_F(GdbwireStringTest, appendCstrDataMixed)
{
    std::string expected;

    // Append a character and check the state
    // Notice the NULL character isn't included in the size
    expected = "a";
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "a"));
    validate(string, 1, 128, expected);

    // Append a character and check the state
    // Notice the NULL character is included in the size
    expected.append("", 1);
    ASSERT_EQ(0, gdbwire_string_append_data(string, "", 1));
    validate(string, 2, 128, expected);

    // Append an empty string and check the state
    // Notice the original NULL character is in the string but a new
    // one was not added
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, ""));
    validate(string, 2, 128, expected);

    // Append another character and check the state
    // Notice the original NULL character is still included in the size
    expected += "a";
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "a"));
    validate(string, 3, 128, expected);
}

TEST_F(GdbwireStringTest, clear_null_instance)
{
    gdbwire_string_clear(NULL);
}

TEST_F(GdbwireStringTest, clear)
{
    size_t non_default_capacity = 8192;

    // Append a longer string and check the state
    std::string longstr(8000, 'a');
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, longstr.c_str()));
    validate(string, 8000, non_default_capacity, longstr);

    gdbwire_string_clear(string);

    // State after clear - the string is now an empty string
    // The capacity is remains unchanged
    validate(string, 0, non_default_capacity, "");
}

TEST_F(GdbwireStringTest, capacity)
{
    // The algorithm is documented internally as follows:
    // The algorithm chosen to increase the capacity is arbitrary.
    // It starts at 128 bytes. It then doubles it's size in bytes like this,
    //   128, 256, 512, 1024, 2048, 4096
    // After it reaches 4096 it then grows by 4096 bytes at a time.

    for (int i = 1; i <= 4096; ++i) {

        gdbwire_string_append_cstr(string, "a");

        switch (i) {
            case 1 ... 127:
                ASSERT_EQ((size_t)128, gdbwire_string_capacity(string));
                break;
            case 128 ... 255:
                ASSERT_EQ((size_t)256, gdbwire_string_capacity(string));
                break;
            case 256 ... 511:
                ASSERT_EQ((size_t)512, gdbwire_string_capacity(string));
                break;
            case 512 ... 1023:
                ASSERT_EQ((size_t)1024, gdbwire_string_capacity(string));
                break;
            case 1024 ... 2047:
                ASSERT_EQ((size_t)2048, gdbwire_string_capacity(string));
                break;
            case 2048 ... 4095:
                ASSERT_EQ((size_t)4096, gdbwire_string_capacity(string));
                break;
            case 4096:
                ASSERT_EQ((size_t)8192, gdbwire_string_capacity(string));
                break;
            default:
                FAIL();
                break;
        }
    }
}

TEST_F(GdbwireStringTest, find_first_of_null_instance)
{
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(NULL, NULL));
}

TEST_F(GdbwireStringTest, find_first_of_empty_string_instance)
{
    // empty string instance always returns position 0 which is
    // gdbwire_string_size().
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, NULL));
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, ""));
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, "a"));
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, "abc"));
}

TEST_F(GdbwireStringTest, find_first_of)
{
    size_t size = 17;
    std::string expected("abcdeabcde\0abcdef", size);
    ASSERT_EQ(size, expected.size());

    // Set up the string instance to be searched
    ASSERT_EQ(0, gdbwire_string_append_data(
            string, expected.data(), expected.size()));
    validate(string, expected.size(), 128, expected);

    // An empty string fails to match
    ASSERT_EQ(size, gdbwire_string_find_first_of(string, ""));

    // Searching for 'a' finds the first position of a.
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, "a"));

    // Searching for 'e' finds the first position of e.
    ASSERT_EQ((size_t)4, gdbwire_string_find_first_of(string, "e"));

    // Searching for 'a' or 'e' finds the first position of a.
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, "ae"));

    // Searching for 'e' or 'a' finds the first position of a.
    ASSERT_EQ((size_t)0, gdbwire_string_find_first_of(string, "ea"));

    // Searching for 'f' finds the first position of f (after NUL char).
    ASSERT_EQ(size - 1, gdbwire_string_find_first_of(string, "f"));
}

TEST_F(GdbwireStringTest, erase_null_instance)
{
    ASSERT_EQ(-1, gdbwire_string_erase(NULL, 0, 0));
}

TEST_F(GdbwireStringTest, erase_empty_string_instance)
{
    ASSERT_EQ(-1, gdbwire_string_erase(string, 0, 0));
    validate(string, 0, 128, ""); 

    ASSERT_EQ(-1, gdbwire_string_erase(string, 0, 2));
    validate(string, 0, 128, ""); 

    ASSERT_EQ(-1, gdbwire_string_erase(string, 2, 0));
    validate(string, 0, 128, ""); 

    ASSERT_EQ(-1, gdbwire_string_erase(string, 2, 2));
    validate(string, 0, 128, ""); 
}

TEST_F(GdbwireStringTest, erase_entire_string_instance)
{
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "hello"));
    validate(string, 5, 128, "hello");

    ASSERT_EQ(0, gdbwire_string_erase(string, 0, 5));
    validate(string, 0, 128, ""); 
}

TEST_F(GdbwireStringTest, erase_count_past_size)
{
    // Setup string
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "hello"));
    validate(string, 5, 128, "hello");

    // entire string
    ASSERT_EQ(0, gdbwire_string_erase(string, 0, 10));
    validate(string, 0, 128, ""); 

    // Setup string
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "hello"));
    validate(string, 5, 128, "hello");

    // erase starting at position 1
    ASSERT_EQ(0, gdbwire_string_erase(string, 1, 10));
    validate(string, 1, 128, "h"); 

    // Setup string
    gdbwire_string_clear(string);
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "hello"));
    validate(string, 5, 128, "hello");

    // erase starting at last position
    ASSERT_EQ(0, gdbwire_string_erase(string, 4, 10));
    validate(string, 4, 128, "hell"); 
}

TEST_F(GdbwireStringTest, erase_pos_past_size)
{
    // Setup string
    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "hello"));
    validate(string, 5, 128, "hello");

    // entire string
    ASSERT_EQ(-1, gdbwire_string_erase(string, 5, 0));
    validate(string, 5, 128, "hello");
}

TEST_F(GdbwireStringTest, erase_string_instance)
{
    size_t pos, count;

    ASSERT_EQ(0, gdbwire_string_append_cstr(string, "abc"));
    validate(string, 3, 128, "abc");

    for (pos = 0; pos < 3; ++pos) {
        for (count = 0; count < 4; ++count) {
            gdbwire_string_clear(string);
            ASSERT_EQ(0, gdbwire_string_append_cstr(string, "abc"));
            validate(string, 3, 128, "abc");

            if (pos == 0) {
                static const char *result[] = { "abc", "bc", "c", "" };
                ASSERT_EQ(0, gdbwire_string_erase(string, pos, count));
                validate(string, strlen(result[count]), 128, result[count]);
            } else if (pos == 1) {
                static const char *result[] = { "abc", "ac", "a", "a" };
                ASSERT_EQ(0, gdbwire_string_erase(string, pos, count));
                validate(string, strlen(result[count]), 128, result[count]);
            } else if (pos == 2) {
                static const char *result[] = { "abc", "ab", "ab", "ab" };
                ASSERT_EQ(0, gdbwire_string_erase(string, pos, count));
                validate(string, strlen(result[count]), 128, result[count]);
            } else {
                ASSERT_EQ(-1, gdbwire_string_erase(string, pos, count));
            }
        }
    }
}
