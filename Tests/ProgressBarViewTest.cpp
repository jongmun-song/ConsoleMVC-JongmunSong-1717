// Phase 2-A - ProgressBarView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - 0%, 50%, 100% ratios produce the expected bar/percentage text
//   - out-of-range ratios are clamped (negative -> 0%, >1.0 -> 100%)

#include <gtest/gtest.h>

#include "../View/ProgressBarView.h"

namespace
{
    using ConsoleMVC::View::ProgressBarView;
}

TEST(ProgressBarViewTest, ZeroRatioRendersEmptyBarAndZeroPercent)
{
    const std::string result = ProgressBarView::Render(0.0, 10);

    EXPECT_EQ(result, "[----------] 0%");
}

TEST(ProgressBarViewTest, FullRatioRendersFullBarAndHundredPercent)
{
    const std::string result = ProgressBarView::Render(1.0, 10);

    EXPECT_EQ(result, "[##########] 100%");
}

TEST(ProgressBarViewTest, HalfRatioRendersHalfFilledBarAndFiftyPercent)
{
    const std::string result = ProgressBarView::Render(0.5, 10);

    EXPECT_EQ(result, "[#####-----] 50%");
}

TEST(ProgressBarViewTest, NegativeRatioClampsToZeroPercent)
{
    const std::string result = ProgressBarView::Render(-0.5, 10);

    EXPECT_EQ(result, "[----------] 0%");
}

TEST(ProgressBarViewTest, RatioAboveOneClampsToHundredPercent)
{
    const std::string result = ProgressBarView::Render(1.5, 10);

    EXPECT_EQ(result, "[##########] 100%");
}

TEST(ProgressBarViewTest, DefaultBarWidthIsTwentyCharacters)
{
    const std::string result = ProgressBarView::Render(1.0);

    EXPECT_EQ(result, "[####################] 100%");
}
