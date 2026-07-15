#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// ProductionLineView adapts Example::Model::ProductionQueueEntry into a
// TableView list where each row is badged (BadgeView) with its production
// status and shown alongside a ProgressBarView gauge. ProductionState's
// three sequential steps (WAITING -> PRODUCING -> CONFIRMED) are mapped to
// progress/badge explicitly via a switch (no default case), so a compiler
// warning (-Wswitch) flags any future ProductionState addition that this
// adapter forgets to handle:
//   WAITING   -> 0%   progress, Normal badge  (not yet started - nothing to
//                warn about)
//   PRODUCING -> 50%  progress, Warning badge (actively running - draws
//                attention while incomplete)
//   CONFIRMED -> 100% progress, Normal badge  (finished successfully)
// This progress/badge mapping is this adapter's own design decision, not a
// core View concept.

#include "../Model/ProductionQueueEntry.h"
#include "FormattingUtil.h"
#include "../../View/TableView.h"
#include "../../View/BadgeView.h"
#include "../../View/ProgressBarView.h"

#include <cstddef>
#include <iostream>
#include <stdexcept>
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
            switch (state)
            {
            case Model::ProductionState::WAITING:
                return 0.0;
            case Model::ProductionState::PRODUCING:
                return 0.5;
            case Model::ProductionState::CONFIRMED:
                return 1.0;
            }
            throw std::logic_error("ProductionLineView::ProgressRatio: unhandled ProductionState");
        }

        static std::string StatusBadge(Model::ProductionState state)
        {
            switch (state)
            {
            case Model::ProductionState::WAITING:
                return ConsoleMVC::View::BadgeView::Render("WAITING", ConsoleMVC::View::BadgeStyle::Normal);
            case Model::ProductionState::PRODUCING:
                return ConsoleMVC::View::BadgeView::Render("PRODUCING", ConsoleMVC::View::BadgeStyle::Warning);
            case Model::ProductionState::CONFIRMED:
                return ConsoleMVC::View::BadgeView::Render("CONFIRMED", ConsoleMVC::View::BadgeStyle::Normal);
            }
            throw std::logic_error("ProductionLineView::StatusBadge: unhandled ProductionState");
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
