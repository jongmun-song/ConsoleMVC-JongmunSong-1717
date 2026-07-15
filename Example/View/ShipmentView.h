#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// ShipmentView renders the release confirmation prompt (ConfirmView) and
// the resulting outcome message (MessageView) for the shipment/release
// step. It takes plain fields rather than an Example::Model type because
// a shipment is a one-shot action on an existing Order/Sample pair, not a
// stored entity of its own.

#include "../../View/ConfirmView.h"
#include "../../View/MessageView.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class ShipmentView
    {
    public:
        static std::string RenderConfirmation(
            int orderId,
            const std::string& sampleName,
            int quantity,
            const std::string& prompt = "Release this shipment? (Y/N) > ")
        {
            const std::vector<std::pair<std::string, std::string>> summary = {
                { "Order ID", std::to_string(orderId) },
                { "Sample", sampleName },
                { "Quantity", std::to_string(quantity) }
            };
            return ConsoleMVC::View::ConfirmView::Render(summary, prompt);
        }

        static void PrintConfirmation(
            int orderId,
            const std::string& sampleName,
            int quantity,
            const std::string& prompt = "Release this shipment? (Y/N) > ")
        {
            std::cout << RenderConfirmation(orderId, sampleName, quantity, prompt);
        }

        static std::string RenderResult(bool succeeded, const std::string& message)
        {
            return ConsoleMVC::View::MessageView::Render(
                message,
                succeeded ? ConsoleMVC::View::MessageStyle::Success : ConsoleMVC::View::MessageStyle::Failure);
        }

        static void PrintResult(bool succeeded, const std::string& message)
        {
            std::cout << RenderResult(succeeded, message) << "\n";
        }
    };
}
