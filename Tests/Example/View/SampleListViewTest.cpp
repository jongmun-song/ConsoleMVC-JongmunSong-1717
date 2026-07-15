// Example/View/SampleListView.h contract tests (PoC verification domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// SampleListView adapts Sample -> TableView columns: ID/Name/Avg. Prod.
// Time/Yield/Stock.

#include <gtest/gtest.h>

#include "../../../Example/View/SampleListView.h"

namespace
{
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Example::View::SampleListView;
}

TEST(SampleListViewTest, RenderContainsEachSampleFieldForEachSample)
{
    const std::vector<Sample> samples = {
        Sample(1, "Alpha", 2.5, 0.9, 10),
        Sample(2, "Beta", 1.0, 0.8, 3),
    };

    const std::string result = SampleListView::Render(samples);

    EXPECT_NE(result.find("1"), std::string::npos);
    EXPECT_NE(result.find("Alpha"), std::string::npos);
    EXPECT_NE(result.find("2.50"), std::string::npos);
    EXPECT_NE(result.find("90%"), std::string::npos);
    EXPECT_NE(result.find("10"), std::string::npos);

    EXPECT_NE(result.find("2"), std::string::npos);
    EXPECT_NE(result.find("Beta"), std::string::npos);
    EXPECT_NE(result.find("1.00"), std::string::npos);
    EXPECT_NE(result.find("80%"), std::string::npos);
    EXPECT_NE(result.find("3"), std::string::npos);
}

TEST(SampleListViewTest, RenderPreservesSampleOrder)
{
    const std::vector<Sample> samples = {
        Sample(1, "Alpha", 2.5, 0.9, 10),
        Sample(2, "Beta", 1.0, 0.8, 3),
    };

    const std::string result = SampleListView::Render(samples);

    EXPECT_LT(result.find("Alpha"), result.find("Beta"));
}

TEST(SampleListViewTest, RenderContainsColumnHeaders)
{
    const std::vector<Sample> samples = { Sample(1, "Alpha", 2.5, 0.9, 10) };

    const std::string result = SampleListView::Render(samples);

    EXPECT_NE(result.find("ID"), std::string::npos);
    EXPECT_NE(result.find("Name"), std::string::npos);
    EXPECT_NE(result.find("Avg. Prod. Time"), std::string::npos);
    EXPECT_NE(result.find("Yield"), std::string::npos);
    EXPECT_NE(result.find("Stock"), std::string::npos);
}
