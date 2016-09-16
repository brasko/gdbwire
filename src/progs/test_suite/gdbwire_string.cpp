#include <string.h>
#include "catch.hpp"
#include "fixture.h"
#include "gdbwire_string.h"

namespace {
    struct GdbwireStringTest : public Fixture {
        GdbwireStringTest() {
            string = gdbwire_string_create();
            REQUIRE(string);
        }

        ~GdbwireStringTest() {
            gdbwire_string_destroy(string);
        }

        void validate(gdbwire_string *instance, size_t size,
                size_t capacity, const std::string &data) {
            std::string actual = std::string(gdbwire_string_data(instance),
                    gdbwire_string_size(instance));

            REQUIRE(size == gdbwire_string_size(instance));
            REQUIRE(capacity == gdbwire_string_capacity(instance));
            REQUIRE(data == actual);
        }

        gdbwire_string *string;
    };
}

TEST_CASE_METHOD_N(GdbwireStringTest, destroy/null_instance)
{
    gdbwire_string_destroy(NULL);
}

TEST_CASE_METHOD_N(GdbwireStringTest, validateInitialState)
{
    validate(string, 0, 128, "");
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_cstr/null_value)
{
    REQUIRE(gdbwire_string_append_cstr(string, NULL) == -1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_cstr/null_instance)
{
    REQUIRE(gdbwire_string_append_cstr(NULL, "hi") == -1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_cstr/standard)
{
    // Append empty string to emptry string and check the state
    REQUIRE(gdbwire_string_append_cstr(string, "") == 0);
    validate(string, 0, 128, "");

    // Append a character and check the state
    REQUIRE(gdbwire_string_append_cstr(string, "a") == 0);
    validate(string, 1, 128, "a");

    // Append empty string to non empty string and check the state
    REQUIRE(gdbwire_string_append_cstr(string, "") == 0);
    validate(string, 1, 128, "a");

    // Append another a character and check the state
    REQUIRE(gdbwire_string_append_cstr(string, "a") == 0);
    validate(string, 2, 128, "aa");

    // Append a longer string and check the state
    REQUIRE(gdbwire_string_append_cstr(string, "abc") == 0);
    validate(string, 5, 128, "aaabc");

    // Append until size is 127 + a NULL character
    std::string longstring(122, 'd');
    std::string expected = std::string("aaabc") + longstring;

    REQUIRE(gdbwire_string_append_cstr(string, longstring.c_str()) == 0);
    validate(string, 127, 128, expected);

    // Append just one more, to push up capacity
    expected += "e";

    REQUIRE(gdbwire_string_append_cstr(string, "e") == 0);
    validate(string, 128, 256, expected);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_data/null_value)
{
    REQUIRE(gdbwire_string_append_data(string, NULL, 0) == -1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_data/null_instance)
{
    REQUIRE(gdbwire_string_append_data(NULL, "a", 1) == -1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_data/standard)
{
    std::string expected;

    // Append empty string to emptry string and check the state
    REQUIRE(gdbwire_string_append_data(string, "", 0) == 0);
    validate(string, 0, 128, expected);

    // Append a character and check the state
    expected.append("a", 1);
    REQUIRE(gdbwire_string_append_data(string, "a", 1) == 0);
    validate(string, 1, 128, expected);

    // Append empty string to a non emptry string and check the state
    REQUIRE(gdbwire_string_append_data(string, "", 0) == 0);
    validate(string, 1, 128, expected);

    // Append another string with binary data  and check the state
    expected.append("a", 2);
    REQUIRE(gdbwire_string_append_data(string, "a", 2) == 0);
    validate(string, 3, 128, expected);

    // Ensure the NULL character was written to the string
    REQUIRE(gdbwire_string_data(string)[2] == '\0');

    // Append another a character and check the state
    expected.append("ad", 2);
    REQUIRE(gdbwire_string_append_data(string, "ad", 2) == 0);
    validate(string, 5, 128, expected);

    // Append until size is 128 with no trailing NULL character
    std::string longstring(123, 'd');
    expected += longstring;

    REQUIRE(gdbwire_string_append_data(
            string, longstring.data(), longstring.size()) == 0);
    validate(string, 128, 128, expected);

    // Append just one more, to push up capacity
    expected += "e";

    REQUIRE(gdbwire_string_append_data(string, "e", 1) == 0);
    validate(string, 129, 256, expected);
}

TEST_CASE_METHOD_N(GdbwireStringTest, append_cstr/mixed)
{
    std::string expected;

    // Append a character and check the state
    // Notice the NULL character isn't included in the size
    expected = "a";
    REQUIRE(gdbwire_string_append_cstr(string, "a") == 0);
    validate(string, 1, 128, expected);

    // Append a character and check the state
    // Notice the NULL character is included in the size
    expected.append("", 1);
    REQUIRE(gdbwire_string_append_data(string, "", 1) == 0);
    validate(string, 2, 128, expected);

    // Append an empty string and check the state
    // Notice the original NULL character is in the string but a new
    // one was not added
    REQUIRE(gdbwire_string_append_cstr(string, "") == 0);
    validate(string, 2, 128, expected);

    // Append another character and check the state
    // Notice the original NULL character is still included in the size
    expected += "a";
    REQUIRE(gdbwire_string_append_cstr(string, "a") == 0);
    validate(string, 3, 128, expected);
}

TEST_CASE_METHOD_N(GdbwireStringTest, clear/null_instance)
{
    gdbwire_string_clear(NULL);
}

TEST_CASE_METHOD_N(GdbwireStringTest, clear/standard)
{
    size_t non_default_capacity = 8192;

    // Append a longer string and check the state
    std::string longstr(8000, 'a');
    REQUIRE(gdbwire_string_append_cstr(string, longstr.c_str()) == 0);
    validate(string, 8000, non_default_capacity, longstr);

    gdbwire_string_clear(string);

    // State after clear - the string is now an empty string
    // The capacity is remains unchanged
    validate(string, 0, non_default_capacity, "");
}

TEST_CASE_METHOD_N(GdbwireStringTest, capacity)
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
                REQUIRE(gdbwire_string_capacity(string) == (size_t)128);
                break;
            case 128 ... 255:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)256);
                break;
            case 256 ... 511:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)512);
                break;
            case 512 ... 1023:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)1024);
                break;
            case 1024 ... 2047:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)2048);
                break;
            case 2048 ... 4095:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)4096);
                break;
            case 4096:
                REQUIRE(gdbwire_string_capacity(string) == (size_t)8192);
                break;
            default:
                FAIL("Should be unreachable");
                break;
        }
    }
}

TEST_CASE_METHOD_N(GdbwireStringTest, find_first_of/null_instance)
{
    REQUIRE(gdbwire_string_find_first_of(NULL, NULL) == (size_t)0);
}

TEST_CASE_METHOD_N(GdbwireStringTest, find_first_of/empty_string_instance)
{
    // empty string instance always returns position 0 which is
    // gdbwire_string_size().
    REQUIRE(gdbwire_string_find_first_of(string, NULL) == (size_t)0);
    REQUIRE(gdbwire_string_find_first_of(string, "") == (size_t)0);
    REQUIRE(gdbwire_string_find_first_of(string, "a") == (size_t)0);
    REQUIRE(gdbwire_string_find_first_of(string, "abc") == (size_t)0);
}

TEST_CASE_METHOD_N(GdbwireStringTest, find_first_of/standard)
{
    size_t size = 17;
    std::string expected("abcdeabcde\0abcdef", size);
    REQUIRE(expected.size() == size);

    // Set up the string instance to be searched
    REQUIRE(gdbwire_string_append_data(
            string, expected.data(), expected.size()) == 0);
    validate(string, expected.size(), 128, expected);

    // An empty string fails to match
    REQUIRE(gdbwire_string_find_first_of(string, "") == size);

    // Searching for 'a' finds the first position of a.
    REQUIRE(gdbwire_string_find_first_of(string, "a") == (size_t)0);

    // Searching for 'e' finds the first position of e.
    REQUIRE(gdbwire_string_find_first_of(string, "e") == (size_t)4);

    // Searching for 'a' or 'e' finds the first position of a.
    REQUIRE(gdbwire_string_find_first_of(string, "ae") == (size_t)0);

    // Searching for 'e' or 'a' finds the first position of a.
    REQUIRE(gdbwire_string_find_first_of(string, "ea") == (size_t)0);

    // Searching for 'f' finds the first position of f (after NUL char).
    REQUIRE(gdbwire_string_find_first_of(string, "f") == size - 1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/null_instance)
{
    REQUIRE(gdbwire_string_erase(NULL, 0, 0) == -1);
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/empty_string_instance)
{
    REQUIRE(gdbwire_string_erase(string, 0, 0) == -1);
    validate(string, 0, 128, ""); 

    REQUIRE(gdbwire_string_erase(string, 0, 2) == -1);
    validate(string, 0, 128, ""); 

    REQUIRE(gdbwire_string_erase(string, 2, 0) == -1);
    validate(string, 0, 128, ""); 

    REQUIRE(gdbwire_string_erase(string, 2, 2) == -1);
    validate(string, 0, 128, ""); 
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/entire_string_instance)
{
    REQUIRE(gdbwire_string_append_cstr(string, "hello") == 0);
    validate(string, 5, 128, "hello");

    REQUIRE(gdbwire_string_erase(string, 0, 5) == 0);
    validate(string, 0, 128, ""); 
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/count_past_size)
{
    // Setup string
    REQUIRE(gdbwire_string_append_cstr(string, "hello") == 0);
    validate(string, 5, 128, "hello");

    // entire string
    REQUIRE(gdbwire_string_erase(string, 0, 10) == 0);
    validate(string, 0, 128, ""); 

    // Setup string
    REQUIRE(gdbwire_string_append_cstr(string, "hello") == 0);
    validate(string, 5, 128, "hello");

    // erase starting at position 1
    REQUIRE(gdbwire_string_erase(string, 1, 10) == 0);
    validate(string, 1, 128, "h"); 

    // Setup string
    gdbwire_string_clear(string);
    REQUIRE(gdbwire_string_append_cstr(string, "hello") == 0);
    validate(string, 5, 128, "hello");

    // erase starting at last position
    REQUIRE(gdbwire_string_erase(string, 4, 10) == 0);
    validate(string, 4, 128, "hell"); 
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/pos_past_size)
{
    // Setup string
    REQUIRE(gdbwire_string_append_cstr(string, "hello") == 0);
    validate(string, 5, 128, "hello");

    // entire string
    REQUIRE(gdbwire_string_erase(string, 5, 0) == -1);
    validate(string, 5, 128, "hello");
}

TEST_CASE_METHOD_N(GdbwireStringTest, erase/standard)
{
    size_t pos, count;

    REQUIRE(gdbwire_string_append_cstr(string, "abc") == 0);
    validate(string, 3, 128, "abc");

    for (pos = 0; pos < 3; ++pos) {
        for (count = 0; count < 4; ++count) {
            gdbwire_string_clear(string);
            REQUIRE(gdbwire_string_append_cstr(string, "abc") == 0);
            validate(string, 3, 128, "abc");

            if (pos == 0) {
                static const char *result[] = { "abc", "bc", "c", "" };
                REQUIRE(gdbwire_string_erase(string, pos, count) == 0);
                validate(string, strlen(result[count]), 128, result[count]);
            } else if (pos == 1) {
                static const char *result[] = { "abc", "ac", "a", "a" };
                REQUIRE(gdbwire_string_erase(string, pos, count) == 0);
                validate(string, strlen(result[count]), 128, result[count]);
            } else if (pos == 2) {
                static const char *result[] = { "abc", "ab", "ab", "ab" };
                REQUIRE(gdbwire_string_erase(string, pos, count) == 0);
                validate(string, strlen(result[count]), 128, result[count]);
            } else {
                REQUIRE(gdbwire_string_erase(string, pos, count) == -1);
            }
        }
    }
}
