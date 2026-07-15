// Phase 3 - InputReader contract tests.
//
// docs/feature/controller.md section 4 (exception/error handling table):
//   - non-numeric input -> parse failure
//   - blank input (Enter only) -> parse failure (no default value)
//   - Y/N confirmation answers other than Y/N -> parse failure
//   - all of the above are retried, never thrown, via ReadUntilValid

#include <gtest/gtest.h>

#include "../Controller/InputReader.h"
#include "Support/ConsoleRedirect.h"

namespace
{
    using ConsoleMVC::Controller::InputReader;
    using ConsoleMVC::Tests::ScopedStdinRedirect;
}

TEST(InputReaderTest, TrimRemovesLeadingAndTrailingWhitespace)
{
    EXPECT_EQ(InputReader::Trim("  hello  "), "hello");
    EXPECT_EQ(InputReader::Trim("no-padding"), "no-padding");
    EXPECT_EQ(InputReader::Trim("   "), "");
    EXPECT_EQ(InputReader::Trim(""), "");
}

TEST(InputReaderTest, ParseIntAcceptsWholeNumberStrings)
{
    EXPECT_EQ(InputReader::ParseInt("42"), 42);
    EXPECT_EQ(InputReader::ParseInt("0"), 0);
    EXPECT_EQ(InputReader::ParseInt("-3"), -3);
}

TEST(InputReaderTest, ParseIntRejectsBlankInput)
{
    EXPECT_FALSE(InputReader::ParseInt("").has_value());
}

TEST(InputReaderTest, ParseIntRejectsNonNumericInput)
{
    EXPECT_FALSE(InputReader::ParseInt("abc").has_value());
}

TEST(InputReaderTest, ParseIntRejectsPartiallyNumericInput)
{
    EXPECT_FALSE(InputReader::ParseInt("12abc").has_value());
}

TEST(InputReaderTest, ParseYesNoAcceptsCaseInsensitiveYAndN)
{
    EXPECT_EQ(InputReader::ParseYesNo("Y"), true);
    EXPECT_EQ(InputReader::ParseYesNo("y"), true);
    EXPECT_EQ(InputReader::ParseYesNo("N"), false);
    EXPECT_EQ(InputReader::ParseYesNo("n"), false);
}

TEST(InputReaderTest, ParseYesNoRejectsAnythingElse)
{
    EXPECT_FALSE(InputReader::ParseYesNo("").has_value());
    EXPECT_FALSE(InputReader::ParseYesNo("yes").has_value());
    EXPECT_FALSE(InputReader::ParseYesNo("x").has_value());
}

TEST(InputReaderTest, ReadUntilValidReturnsFirstValidLineWithoutRetrying)
{
    ScopedStdinRedirect stdinRedirect("7\n");

    int invalidCallCount = 0;
    const int result = InputReader::ReadUntilValid<int>(
        &InputReader::ParseInt,
        [&invalidCallCount] { ++invalidCallCount; });

    EXPECT_EQ(result, 7);
    EXPECT_EQ(invalidCallCount, 0);
}

TEST(InputReaderTest, ReadUntilValidRetriesOnBlankAndNonNumericInput)
{
    // Blank line, then non-numeric, then a valid number.
    ScopedStdinRedirect stdinRedirect("\nabc\n9\n");

    int invalidCallCount = 0;
    const int result = InputReader::ReadUntilValid<int>(
        &InputReader::ParseInt,
        [&invalidCallCount] { ++invalidCallCount; });

    EXPECT_EQ(result, 9);
    EXPECT_EQ(invalidCallCount, 2);
}
