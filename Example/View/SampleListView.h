#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// SampleListView adapts a collection of Example::Model::Sample into the
// generic (headers, rows) shape ConsoleMVC::View::TableView renders. It
// contains no rendering logic of its own - it only decides which Sample
// fields become which table column, then delegates to TableView.

#include "../Model/Sample.h"
#include "FormattingUtil.h"
#include "../../View/TableView.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class SampleListView
    {
    public:
        static std::string Render(
            const std::vector<Model::Sample>& samples,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            static const std::vector<std::string> headers = {
                "ID", "Name", "Avg. Prod. Time", "Yield", "Stock"
            };

            return ConsoleMVC::View::TableView::Render(headers, ToRows(samples), pageSize, currentPage);
        }

        static void Print(
            const std::vector<Model::Sample>& samples,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            std::cout << Render(samples, pageSize, currentPage);
        }

    private:
        static std::vector<std::vector<std::string>> ToRows(const std::vector<Model::Sample>& samples)
        {
            std::vector<std::vector<std::string>> rows;
            rows.reserve(samples.size());
            for (const auto& sample : samples)
            {
                rows.push_back({
                    std::to_string(sample.GetId()),
                    sample.GetName(),
                    Detail::FormatFixed(sample.GetAverageProductionTimePerUnit()),
                    Detail::FormatPercentage(sample.GetYieldRatio()),
                    std::to_string(sample.GetStockQuantity())
                });
            }
            return rows;
        }
    };
}
