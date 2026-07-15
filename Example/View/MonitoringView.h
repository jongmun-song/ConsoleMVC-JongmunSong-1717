#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// MonitoringView adapts Example::Model::Sample stock levels, plus the
// caller-aggregated open order quantity per sample, into a HeaderView
// (timestamped title) followed by a TableView badged (via BadgeView) with
// a stock-vs-demand status. The stock/order comparison and its badge
// mapping are this adapter's own design decision.

#include "../Model/Sample.h"
#include "../../View/HeaderView.h"
#include "../../View/TableView.h"
#include "../../View/BadgeView.h"

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class MonitoringView
    {
    public:
        // `orderedQuantityBySampleId` is the caller-aggregated total open
        // order quantity per sample id (absent entries are treated as 0
        // outstanding demand).
        static std::string Render(
            const std::string& timestamp,
            const std::vector<Model::Sample>& samples,
            const std::map<int, int>& orderedQuantityBySampleId,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            static const std::vector<std::string> headers = {
                "ID", "Name", "Stock", "Ordered", "Status"
            };

            std::string rendered = ConsoleMVC::View::HeaderView::Render("Monitoring", timestamp);
            rendered += ConsoleMVC::View::TableView::Render(
                headers, ToRows(samples, orderedQuantityBySampleId), pageSize, currentPage);
            return rendered;
        }

        static void Print(
            const std::string& timestamp,
            const std::vector<Model::Sample>& samples,
            const std::map<int, int>& orderedQuantityBySampleId,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            std::cout << Render(timestamp, samples, orderedQuantityBySampleId, pageSize, currentPage);
        }

    private:
        static int OrderedQuantityFor(const std::map<int, int>& orderedQuantityBySampleId, int sampleId)
        {
            const auto found = orderedQuantityBySampleId.find(sampleId);
            return found == orderedQuantityBySampleId.end() ? 0 : found->second;
        }

        // Depleted (Error) once stock is exhausted, short (Warning) once
        // stock can no longer cover outstanding demand, otherwise
        // sufficient (Normal).
        static std::string StockStatusBadge(int stockQuantity, int orderedQuantity)
        {
            if (stockQuantity <= 0)
            {
                return ConsoleMVC::View::BadgeView::Render("DEPLETED", ConsoleMVC::View::BadgeStyle::Error);
            }
            if (stockQuantity < orderedQuantity)
            {
                return ConsoleMVC::View::BadgeView::Render("SHORT", ConsoleMVC::View::BadgeStyle::Warning);
            }
            return ConsoleMVC::View::BadgeView::Render("SUFFICIENT", ConsoleMVC::View::BadgeStyle::Normal);
        }

        static std::vector<std::vector<std::string>> ToRows(
            const std::vector<Model::Sample>& samples,
            const std::map<int, int>& orderedQuantityBySampleId)
        {
            std::vector<std::vector<std::string>> rows;
            rows.reserve(samples.size());
            for (const auto& sample : samples)
            {
                const int orderedQuantity = OrderedQuantityFor(orderedQuantityBySampleId, sample.GetId());
                rows.push_back({
                    std::to_string(sample.GetId()),
                    sample.GetName(),
                    std::to_string(sample.GetStockQuantity()),
                    std::to_string(orderedQuantity),
                    StockStatusBadge(sample.GetStockQuantity(), orderedQuantity)
                });
            }
            return rows;
        }
    };
}
