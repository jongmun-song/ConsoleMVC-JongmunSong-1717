// Phase 2-A - ConfirmView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - key/value summary lines appear in order in the rendered output
//   - custom prompt text is reflected in the output
//
// Uses generic key-value pairs only - ConfirmView has no domain knowledge.

#include <gtest/gtest.h>

#include "../View/ConfirmView.h"

namespace
{
    using ConsoleMVC::View::ConfirmView;
}

TEST(ConfirmViewTest, RenderContainsEachKeyValuePair)
{
    const std::vector<std::pair<std::string, std::string>> summary = {
        { "Field A", "Value A" },
        { "Field B", "Value B" },
    };

    const std::string result = ConfirmView::Render(summary);

    EXPECT_NE(result.find("Field A: Value A"), std::string::npos);
    EXPECT_NE(result.find("Field B: Value B"), std::string::npos);
}

TEST(ConfirmViewTest, RenderPreservesSummaryOrder)
{
    const std::vector<std::pair<std::string, std::string>> summary = {
        { "First", "1" },
        { "Second", "2" },
        { "Third", "3" },
    };

    const std::string result = ConfirmView::Render(summary);

    const auto firstPos = result.find("First");
    const auto secondPos = result.find("Second");
    const auto thirdPos = result.find("Third");
    ASSERT_NE(firstPos, std::string::npos);
    ASSERT_NE(secondPos, std::string::npos);
    ASSERT_NE(thirdPos, std::string::npos);
    EXPECT_LT(firstPos, secondPos);
    EXPECT_LT(secondPos, thirdPos);
}

TEST(ConfirmViewTest, RenderUsesDefaultPromptWhenNotSpecified)
{
    const std::vector<std::pair<std::string, std::string>> summary = { { "Key", "Value" } };

    const std::string result = ConfirmView::Render(summary);

    EXPECT_NE(result.find("Y/N > "), std::string::npos);
}

TEST(ConfirmViewTest, RenderReflectsCustomPrompt)
{
    const std::vector<std::pair<std::string, std::string>> summary = { { "Key", "Value" } };

    const std::string result = ConfirmView::Render(summary, "Confirm (y/n)? ");

    EXPECT_NE(result.find("Confirm (y/n)? "), std::string::npos);
}

TEST(ConfirmViewTest, RenderWithEmptySummaryStillProducesPrompt)
{
    const std::vector<std::pair<std::string, std::string>> summary;

    const std::string result = ConfirmView::Render(summary, "Proceed? ");

    EXPECT_EQ(result, "Proceed? ");
}
