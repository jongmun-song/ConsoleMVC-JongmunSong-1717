// Phase 2-A - BadgeView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - Normal/Warning/Error styles render distinct markers around the label
//
// Uses generic label strings only - BadgeStyle is a generic emphasis
// category, not a domain state enumeration.

#include <gtest/gtest.h>

#include "../View/BadgeView.h"

namespace
{
    using ConsoleMVC::View::BadgeStyle;
    using ConsoleMVC::View::BadgeView;
}

TEST(BadgeViewTest, NormalStyleWrapsLabelWithoutMarker)
{
    const std::string result = BadgeView::Render("Status", BadgeStyle::Normal);

    EXPECT_EQ(result, "[Status]");
}

TEST(BadgeViewTest, WarningStyleAddsWarningMarker)
{
    const std::string result = BadgeView::Render("Status", BadgeStyle::Warning);

    EXPECT_EQ(result, "[! Status]");
}

TEST(BadgeViewTest, ErrorStyleAddsErrorMarker)
{
    const std::string result = BadgeView::Render("Status", BadgeStyle::Error);

    EXPECT_EQ(result, "[!! Status]");
}

TEST(BadgeViewTest, DefaultStyleIsNormal)
{
    const std::string result = BadgeView::Render("Status");

    EXPECT_EQ(result, BadgeView::Render("Status", BadgeStyle::Normal));
}

TEST(BadgeViewTest, EachStyleProducesADistinctOutput)
{
    const std::string normal = BadgeView::Render("Same", BadgeStyle::Normal);
    const std::string warning = BadgeView::Render("Same", BadgeStyle::Warning);
    const std::string error = BadgeView::Render("Same", BadgeStyle::Error);

    EXPECT_NE(normal, warning);
    EXPECT_NE(normal, error);
    EXPECT_NE(warning, error);
}

TEST(BadgeViewTest, LabelTextIsPreservedVerbatim)
{
    const std::string result = BadgeView::Render("Arbitrary Label 123", BadgeStyle::Warning);

    EXPECT_NE(result.find("Arbitrary Label 123"), std::string::npos);
}
