#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// ProductionLineView adapts Example::Model::ProductionQueueEntry into a
// TableView list where each row is badged (BadgeView) with its production
// status and shown alongside a ProgressBarView gauge. ProductionState only
// distinguishes PRODUCING/CONFIRMED (no partial-completion tracking), so
// progress is rendered as binary 0%/100% - this simplification is this
// adapter's own design decision, not a core View concept.

#include "../Model/ProductionQueueEntry.h"
#include "FormattingUtil.h"
#include "../../View/TableView.h"
#include "../../View/BadgeView.h"
#include "../../View/ProgressBarView.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class ProductionLineView
    {
    public:
        static std::string Render(
            const std::vector<Model::ProductionQueueEntry>& queue,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            static const std::vector<std::string> headers = {
                "Order ID", "Sample ID", "Actual Qty", "Prod. Time", "Progress", "Status"
            };

            return ConsoleMVC::View::TableView::Render(headers, ToRows(queue), pageSize, currentPage);
        }

        static void Print(
            const std::vector<Model::ProductionQueueEntry>& queue,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            std::cout << Render(queue, pageSize, currentPage);
        }

    private:
        static double ProgressRatio(Model::ProductionState state)
        {
            return state == Model::ProductionState::CONFIRMED ? 1.0 : 0.0;
        }

        static std::string StatusBadge(Model::ProductionState state)
        {
            return state == Model::ProductionState::CONFIRMED
                ? ConsoleMVC::View::BadgeView::Render("CONFIRMED", ConsoleMVC::View::BadgeStyle::Normal)
                : ConsoleMVC::View::BadgeView::Render("PRODUCING", ConsoleMVC::View::BadgeStyle::Warning);
        }

        static std::vector<std::vector<std::string>> ToRows(
            const std::vector<Model::ProductionQueueEntry>& queue)
        {
            std::vector<std::vector<std::string>> rows;
            rows.reserve(queue.size());
            for (const auto& entry : queue)
            {
                rows.push_back({
                    std::to_string(entry.GetId()),
                    std::to_string(entry.GetSampleId()),
                    std::to_string(entry.GetActualProductionQuantity()),
                    Detail::FormatFixed(entry.GetTotalProductionTime()),
                    ConsoleMVC::View::ProgressBarView::Render(ProgressRatio(entry.GetState())),
                    StatusBadge(entry.GetState())
                });
            }
            return rows;
        }
    };
}
