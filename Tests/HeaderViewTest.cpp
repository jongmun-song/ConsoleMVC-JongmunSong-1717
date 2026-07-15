// Phase 2-A - HeaderView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - title-only vs title+subtitle output both contain the supplied strings
//
// Uses generic strings only - HeaderView is domain-agnostic.

#include <gtest/gtest.h>

#include "../View/HeaderView.h"

namespace
{
    using ConsoleMVC::View::HeaderView;
}

TEST(HeaderViewTest, RenderWithTitleOnlyContainsTitle)
{
    const std::string result = HeaderView::Render("Main Menu");

    EXPECT_NE(result.find("Main Menu"), std::string::npos);
}

TEST(HeaderViewTest, RenderWithTitleOnlyOmitsSubtitleSeparator)
{
    const std::string result = HeaderView::Render("Main Menu");

    // No subtitle was supplied, so no extra content should follow the title
    // before the closing marker.
    EXPECT_EQ(result, "== Main Menu ==\n");
}

TEST(HeaderViewTest, RenderWithTitleAndSubtitleContainsBoth)
{
    const std::string result = HeaderView::Render("Main Menu", "Page 1/3");

    EXPECT_NE(result.find("Main Menu"), std::string::npos);
    EXPECT_NE(result.find("Page 1/3"), std::string::npos);
}

TEST(HeaderViewTest, RenderWithSubtitlePreservesTitleBeforeSubtitle)
{
    const std::string result = HeaderView::Render("Main Menu", "Page 1/3");

    EXPECT_LT(result.find("Main Menu"), result.find("Page 1/3"));
}
