#pragma once

// docs/feature/view.md section 3.3 - table/pagination rendering
//
// TableView renders arbitrary column headers and row data (both plain
// string cells) as an aligned text table. Column names and cell contents
// are always supplied by the caller - TableView has no knowledge of what
// a "column" means for any particular domain.
//
// Pagination is opt-in: pageSize == 0 renders every row on a single page.
// When pageSize > 0, only the rows belonging to `currentPage` (0-based)
// are rendered, and a "more rows follow" notice is appended if further
// pages remain. Column widths are computed from the full row set (not
// just the current page) so that a table looks consistent as a user pages
// through it.

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::View
{
    class TableView
    {
    public:
        // Renders `headers` and the slice of `rows` belonging to
        // `currentPage`. `nextPageNotice` is appended verbatim when more
        // rows remain beyond the rendered page.
        static std::string Render(
            const std::vector<std::string>& headers,
            const std::vector<std::vector<std::string>>& rows,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0,
            const std::string& nextPageNotice = "... more rows available\n")
        {
            const std::vector<std::size_t> columnWidths = ComputeColumnWidths(headers, rows);

            std::ostringstream out;
            AppendRow(out, headers, columnWidths);
            AppendSeparator(out, columnWidths);

            const auto [firstRow, lastRow] = PageBounds(rows.size(), pageSize, currentPage);
            for (std::size_t rowIndex = firstRow; rowIndex < lastRow; ++rowIndex)
            {
                AppendRow(out, rows[rowIndex], columnWidths);
            }

            if (pageSize > 0 && lastRow < rows.size())
            {
                out << nextPageNotice;
            }

            return out.str();
        }

        static void Print(
            const std::vector<std::string>& headers,
            const std::vector<std::vector<std::string>>& rows,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0,
            const std::string& nextPageNotice = "... more rows available\n")
        {
            std::cout << Render(headers, rows, pageSize, currentPage, nextPageNotice);
        }

    private:
        // Computes the [firstRow, lastRow) half-open range of row indices
        // that belong to `currentPage`. pageSize == 0 means "no pagination",
        // so the whole table is a single page.
        static std::pair<std::size_t, std::size_t> PageBounds(
            std::size_t totalRows, std::size_t pageSize, std::size_t currentPage)
        {
            if (pageSize == 0)
            {
                return { 0, totalRows };
            }

            const std::size_t firstRow = std::min(currentPage * pageSize, totalRows);
            const std::size_t lastRow = std::min(firstRow + pageSize, totalRows);
            return { firstRow, lastRow };
        }

        static std::vector<std::size_t> ComputeColumnWidths(
            const std::vector<std::string>& headers,
            const std::vector<std::vector<std::string>>& rows)
        {
            std::vector<std::size_t> widths(headers.size());
            for (std::size_t column = 0; column < headers.size(); ++column)
            {
                widths[column] = headers[column].size();
            }

            for (const auto& row : rows)
            {
                for (std::size_t column = 0; column < row.size() && column < widths.size(); ++column)
                {
                    widths[column] = std::max(widths[column], row[column].size());
                }
            }

            return widths;
        }

        static void AppendRow(
            std::ostringstream& out,
            const std::vector<std::string>& cells,
            const std::vector<std::size_t>& columnWidths)
        {
            for (std::size_t column = 0; column < columnWidths.size(); ++column)
            {
                const std::string& cell = column < cells.size() ? cells[column] : std::string{};
                out << cell << std::string(columnWidths[column] - cell.size(), ' ');
                if (column + 1 < columnWidths.size())
                {
                    out << " | ";
                }
            }
            out << "\n";
        }

        static void AppendSeparator(std::ostringstream& out, const std::vector<std::size_t>& columnWidths)
        {
            for (std::size_t column = 0; column < columnWidths.size(); ++column)
            {
                out << std::string(columnWidths[column], '-');
                if (column + 1 < columnWidths.size())
                {
                    out << "-+-";
                }
            }
            out << "\n";
        }
    };
}
