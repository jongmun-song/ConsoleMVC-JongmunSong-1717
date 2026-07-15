// Phase 2-A - TableView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - headers/rows appear in order in the rendered output
//   - pageSize == 0 disables pagination (all rows on one page, no notice)
//   - pagination boundary values: exactly pageSize rows (no notice) vs.
//     one row beyond pageSize (notice present)
//   - defensive handling of rows with fewer/more cells than headers
//
// Uses generic column/cell strings only - TableView has no domain knowledge.

#include <gtest/gtest.h>

#include "../View/TableView.h"

namespace
{
    using ConsoleMVC::View::TableView;
}

TEST(TableViewTest, RenderContainsHeadersInOrder)
{
    const std::vector<std::string> headers = { "Col A", "Col B" };
    const std::vector<std::vector<std::string>> rows = { { "1", "2" } };

    const std::string result = TableView::Render(headers, rows);

    const auto colAPos = result.find("Col A");
    const auto colBPos = result.find("Col B");
    ASSERT_NE(colAPos, std::string::npos);
    ASSERT_NE(colBPos, std::string::npos);
    EXPECT_LT(colAPos, colBPos);
}

TEST(TableViewTest, RenderContainsAllRowsInOrder)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" },
        { "Row Two" },
        { "Row Three" },
    };

    const std::string result = TableView::Render(headers, rows);

    const auto row1Pos = result.find("Row One");
    const auto row2Pos = result.find("Row Two");
    const auto row3Pos = result.find("Row Three");
    ASSERT_NE(row1Pos, std::string::npos);
    ASSERT_NE(row2Pos, std::string::npos);
    ASSERT_NE(row3Pos, std::string::npos);
    EXPECT_LT(row1Pos, row2Pos);
    EXPECT_LT(row2Pos, row3Pos);
}

TEST(TableViewTest, PageSizeZeroRendersAllRowsOnOnePage)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" }, { "Row Two" }, { "Row Three" }, { "Row Four" },
    };

    const std::string result = TableView::Render(headers, rows, /*pageSize=*/0);

    EXPECT_NE(result.find("Row One"), std::string::npos);
    EXPECT_NE(result.find("Row Four"), std::string::npos);
}

TEST(TableViewTest, PageSizeZeroProducesNoNextPageNotice)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" }, { "Row Two" }, { "Row Three" },
    };

    const std::string result = TableView::Render(
        headers, rows, /*pageSize=*/0, /*currentPage=*/0, "... more rows available\n");

    EXPECT_EQ(result.find("... more rows available"), std::string::npos);
}

TEST(TableViewTest, ExactlyPageSizeRowsProducesNoNextPageNotice)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" }, { "Row Two" },
    };

    const std::string result = TableView::Render(
        headers, rows, /*pageSize=*/2, /*currentPage=*/0, "... more rows available\n");

    EXPECT_EQ(result.find("... more rows available"), std::string::npos);
    EXPECT_NE(result.find("Row One"), std::string::npos);
    EXPECT_NE(result.find("Row Two"), std::string::npos);
}

TEST(TableViewTest, OneRowBeyondPageSizeProducesNextPageNotice)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" }, { "Row Two" }, { "Row Three" },
    };

    const std::string result = TableView::Render(
        headers, rows, /*pageSize=*/2, /*currentPage=*/0, "... more rows available\n");

    EXPECT_NE(result.find("... more rows available"), std::string::npos);
    // Only the first page's rows should be rendered.
    EXPECT_NE(result.find("Row One"), std::string::npos);
    EXPECT_NE(result.find("Row Two"), std::string::npos);
    EXPECT_EQ(result.find("Row Three"), std::string::npos);
}

TEST(TableViewTest, SecondPageRendersRemainingRowWithoutNotice)
{
    const std::vector<std::string> headers = { "Name" };
    const std::vector<std::vector<std::string>> rows = {
        { "Row One" }, { "Row Two" }, { "Row Three" },
    };

    const std::string result = TableView::Render(
        headers, rows, /*pageSize=*/2, /*currentPage=*/1, "... more rows available\n");

    EXPECT_NE(result.find("Row Three"), std::string::npos);
    EXPECT_EQ(result.find("Row One"), std::string::npos);
    EXPECT_EQ(result.find("... more rows available"), std::string::npos);
}

TEST(TableViewTest, RowWithFewerCellsThanHeadersDoesNotCrash)
{
    const std::vector<std::string> headers = { "A", "B", "C" };
    const std::vector<std::vector<std::string>> rows = { { "only-one" } };

    EXPECT_NO_THROW({
        const std::string result = TableView::Render(headers, rows);
        EXPECT_NE(result.find("only-one"), std::string::npos);
    });
}

TEST(TableViewTest, RowWithMoreCellsThanHeadersSilentlyDropsExtras)
{
    const std::vector<std::string> headers = { "A", "B" };
    const std::vector<std::vector<std::string>> rows = { { "cell1", "cell2", "extra-cell" } };

    std::string result;
    EXPECT_NO_THROW({ result = TableView::Render(headers, rows); });

    EXPECT_NE(result.find("cell1"), std::string::npos);
    EXPECT_NE(result.find("cell2"), std::string::npos);
    EXPECT_EQ(result.find("extra-cell"), std::string::npos);
}
