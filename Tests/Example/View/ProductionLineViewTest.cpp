// Example/View/ProductionLineView.h contract tests (PoC verification
// domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// ProductionState only distinguishes PRODUCING/CONFIRMED, so progress is
// rendered as binary 0%/100% (see ProductionLineView::ProgressRatio and
// Tests/ProgressBarViewTest.cpp for the exact bar text each ratio produces).

#include <gtest/gtest.h>

#include "../../../Example/View/ProductionLineView.h"

namespace
{
    using ConsoleMVC::Example::Model::ProductionQueueEntry;
    using ConsoleMVC::Example::Model::ProductionState;
    using ConsoleMVC::Example::View::ProductionLineView;
}

TEST(ProductionLineViewTest, ProducingEntryRendersZeroPercentProgressAndWarningBadge)
{
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);
    ASSERT_EQ(entry.GetState(), ProductionState::PRODUCING);

    const std::string result = ProductionLineView::Render({ entry });

    EXPECT_NE(result.find("] 0%"), std::string::npos);
    EXPECT_NE(result.find("[! PRODUCING]"), std::string::npos);
    EXPECT_EQ(result.find("] 100%"), std::string::npos);
}

TEST(ProductionLineViewTest, ConfirmedEntryRendersHundredPercentProgressAndNormalBadge)
{
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);
    ASSERT_TRUE(entry.TryTransitionTo(ProductionState::CONFIRMED));

    const std::string result = ProductionLineView::Render({ entry });

    EXPECT_NE(result.find("] 100%"), std::string::npos);
    EXPECT_NE(result.find("[CONFIRMED]"), std::string::npos);
    EXPECT_EQ(result.find("] 0%"), std::string::npos);
}

TEST(ProductionLineViewTest, RenderContainsQueueEntryFields)
{
    ProductionQueueEntry entry(7, 100, 50, 112, 0.9, 2.0);

    const std::string result = ProductionLineView::Render({ entry });

    EXPECT_NE(result.find("7"), std::string::npos);
    EXPECT_NE(result.find("100"), std::string::npos);
    EXPECT_NE(result.find(std::to_string(entry.GetActualProductionQuantity())), std::string::npos);
}

TEST(ProductionLineViewTest, RenderContainsMultipleEntriesInOrder)
{
    ProductionQueueEntry producing(1, 100, 10, 20, 0.8, 1.0);
    ProductionQueueEntry confirmed(2, 100, 15, 30, 0.8, 1.0);
    ASSERT_TRUE(confirmed.TryTransitionTo(ProductionState::CONFIRMED));

    const std::string result = ProductionLineView::Render({ producing, confirmed });

    EXPECT_NE(result.find("[! PRODUCING]"), std::string::npos);
    EXPECT_NE(result.find("[CONFIRMED]"), std::string::npos);
    EXPECT_LT(result.find("[! PRODUCING]"), result.find("[CONFIRMED]"));
}
