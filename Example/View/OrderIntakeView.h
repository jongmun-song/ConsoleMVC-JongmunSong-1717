#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// OrderIntakeView renders a not-yet-placed order's summary (sample id /
// customer name / ordered quantity) via ConsoleMVC::View::ConfirmView. No
// Example::Model::Order exists yet at this point (no order id has been
// assigned), so this adapter takes the raw fields a Controller would have
// collected from input rather than requiring a constructed Order.

#include "../../View/ConfirmView.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class OrderIntakeView
    {
    public:
        static std::string RenderConfirmation(
            int sampleId,
            const std::string& customerName,
            int orderedQuantity,
            const std::string& prompt = "Place this order? (Y/N) > ")
        {
            return ConsoleMVC::View::ConfirmView::Render(
                ToSummary(sampleId, customerName, orderedQuantity), prompt);
        }

        static void PrintConfirmation(
            int sampleId,
            const std::string& customerName,
            int orderedQuantity,
            const std::string& prompt = "Place this order? (Y/N) > ")
        {
            std::cout << RenderConfirmation(sampleId, customerName, orderedQuantity, prompt);
        }

    private:
        static std::vector<std::pair<std::string, std::string>> ToSummary(
            int sampleId, const std::string& customerName, int orderedQuantity)
        {
            return {
                { "Sample ID", std::to_string(sampleId) },
                { "Customer", customerName },
                { "Ordered Quantity", std::to_string(orderedQuantity) }
            };
        }
    };
}
