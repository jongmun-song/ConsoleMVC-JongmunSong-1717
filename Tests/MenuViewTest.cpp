// Phase 2-A - MenuView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - numbers/labels appear in order in the rendered output
//   - custom prompt text is reflected in the output
//
// Uses generic menu items only - MenuView does not reserve any number.

#include <gtest/gtest.h>

#include "../View/MenuView.h"

namespace
{
    using ConsoleMVC::View::MenuItem;
    using ConsoleMVC::View::MenuView;
}

TEST(MenuViewTest, RenderContainsEachItemNumberAndLabel)
{
    const std::vector<MenuItem> items = {
        { 1, "First Option" },
        { 2, "Second Option" },
        { 3, "Third Option" },
    };

    const std::string result = MenuView::Render(items);

    EXPECT_NE(result.find("[1] First Option"), std::string::npos);
    EXPECT_NE(result.find("[2] Second Option"), std::string::npos);
    EXPECT_NE(result.find("[3] Third Option"), std::string::npos);
}

TEST(MenuViewTest, RenderPreservesItemOrder)
{
    const std::vector<MenuItem> items = {
        { 9, "Alpha" },
        { 1, "Beta" },
        { 5, "Gamma" },
    };

    const std::string result = MenuView::Render(items);

    const auto alphaPos = result.find("Alpha");
    const auto betaPos = result.find("Beta");
    const auto gammaPos = result.find("Gamma");

    ASSERT_NE(alphaPos, std::string::npos);
    ASSERT_NE(betaPos, std::string::npos);
    ASSERT_NE(gammaPos, std::string::npos);
    EXPECT_LT(alphaPos, betaPos);
    EXPECT_LT(betaPos, gammaPos);
}

TEST(MenuViewTest, RenderUsesDefaultPromptWhenNotSpecified)
{
    const std::vector<MenuItem> items = { { 0, "Back" } };

    const std::string result = MenuView::Render(items);

    EXPECT_NE(result.find("> "), std::string::npos);
}

TEST(MenuViewTest, RenderReflectsCustomPrompt)
{
    const std::vector<MenuItem> items = { { 0, "Back" } };

    const std::string result = MenuView::Render(items, "Select> ");

    EXPECT_NE(result.find("Select> "), std::string::npos);
}

TEST(MenuViewTest, RenderWithEmptyItemsStillProducesPrompt)
{
    const std::vector<MenuItem> items;

    const std::string result = MenuView::Render(items, "Choose: ");

    EXPECT_EQ(result, "Choose: ");
}
