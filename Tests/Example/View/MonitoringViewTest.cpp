// Example/View/MonitoringView.h contract tests (PoC verification domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// ../ref/requirements.pdf Chapter 2 monitoring status semantics (see
// MonitoringView::StockStatusBadge):
//   stock <= 0            -> DEPLETED (Error)  ("고갈")
//   stock <  orderedQty    -> SHORT    (Warning) ("부족")
//   otherwise (stock >= orderedQty, stock > 0) -> SUFFICIENT (Normal) ("여유")
//
// The boundary cases (stock == orderedQty, stock == 0) are the ones most
// likely to regress if the comparison operators are ever swapped, so they
// get dedicated cases below.

#include <gtest/gtest.h>

#include "../../../Example/View/MonitoringView.h"

namespace
{
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Example::View::MonitoringView;
}

TEST(MonitoringViewTest, RenderContainsTimestampInHeader)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 5) };
    const std::map<int, int> ordered = { { 1, 5 } };

    const std::string result = MonitoringView::Render("2026-07-15 10:00:00", samples, ordered);

    EXPECT_NE(result.find("2026-07-15 10:00:00"), std::string::npos);
}

TEST(MonitoringViewTest, ZeroStockIsBadgedDepleted)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 0) };
    const std::map<int, int> ordered = { { 1, 5 } };

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[!! DEPLETED]"), std::string::npos);
}

TEST(MonitoringViewTest, StockBelowOrderedQuantityIsBadgedShort)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 3) };
    const std::map<int, int> ordered = { { 1, 5 } };

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[! SHORT]"), std::string::npos);
}

TEST(MonitoringViewTest, StockAtOrExceedingOrderedQuantityIsBadgedSufficient)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 10) };
    const std::map<int, int> ordered = { { 1, 5 } };

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[SUFFICIENT]"), std::string::npos);
}

// Boundary: stock exactly equal to ordered quantity must be SUFFICIENT
// (Normal), not SHORT - the "short" condition is stock < ordered, so
// equality must fall on the sufficient side.
TEST(MonitoringViewTest, StockExactlyEqualToOrderedQuantityIsBadgedSufficientNotShort)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 5) };
    const std::map<int, int> ordered = { { 1, 5 } };

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[SUFFICIENT]"), std::string::npos);
    EXPECT_EQ(result.find("[! SHORT]"), std::string::npos);
}

// Boundary: stock exactly zero with zero ordered quantity is still
// DEPLETED - depletion is judged on stock alone, independent of demand.
TEST(MonitoringViewTest, ZeroStockWithNoOutstandingOrdersIsStillBadgedDepleted)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 0) };
    const std::map<int, int> ordered;

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[!! DEPLETED]"), std::string::npos);
}

TEST(MonitoringViewTest, SampleWithoutOutstandingOrderEntryIsTreatedAsZeroDemand)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 1.0, 0.9, 5) };
    const std::map<int, int> ordered;

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("[SUFFICIENT]"), std::string::npos);
}

TEST(MonitoringViewTest, RenderContainsMultipleSamplesInOrder)
{
    const std::vector<Sample> samples = {
        Sample(1, "Alpha", 1.0, 0.9, 0),
        Sample(2, "Beta", 1.0, 0.9, 3),
        Sample(3, "Gamma", 1.0, 0.9, 10),
    };
    const std::map<int, int> ordered = { { 1, 1 }, { 2, 5 }, { 3, 5 } };

    const std::string result = MonitoringView::Render("ts", samples, ordered);

    EXPECT_NE(result.find("Alpha"), std::string::npos);
    EXPECT_NE(result.find("Beta"), std::string::npos);
    EXPECT_NE(result.find("Gamma"), std::string::npos);
    EXPECT_LT(result.find("Alpha"), result.find("Beta"));
    EXPECT_LT(result.find("Beta"), result.find("Gamma"));
}
