#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// OrderApprovalView adapts Example::Model::Order into ConsoleMVC::View's
// generic components: a pending-order TableView list badged by status, and
// a ConfirmView summary shown before an approve/reject decision is applied.
// The OrderState -> BadgeStyle mapping below is this adapter's own design
// decision, not something the core BadgeView knows about.

#include "../Model/Order.h"
#include "../../View/TableView.h"
#include "../../View/BadgeView.h"
#include "../../View/ConfirmView.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class OrderApprovalView
    {
    public:
        static std::string RenderPendingOrders(
            const std::vector<Model::Order>& orders,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            static const std::vector<std::string> headers = {
                "Order ID", "Sample ID", "Customer", "Quantity", "Status"
            };

            return ConsoleMVC::View::TableView::Render(headers, ToRows(orders), pageSize, currentPage);
        }

        static void PrintPendingOrders(
            const std::vector<Model::Order>& orders,
            std::size_t pageSize = 0,
            std::size_t currentPage = 0)
        {
            std::cout << RenderPendingOrders(orders, pageSize, currentPage);
        }

        static std::string RenderDecisionConfirmation(
            const Model::Order& order,
            const std::string& prompt = "Approve this order? (Y/N) > ")
        {
            const std::vector<std::pair<std::string, std::string>> summary = {
                { "Order ID", std::to_string(order.GetId()) },
                { "Sample ID", std::to_string(order.GetSampleId()) },
                { "Customer", order.GetCustomerName() },
                { "Quantity", std::to_string(order.GetOrderedQuantity()) },
                { "Current Status", StatusBadge(order.GetState()) }
            };
            return ConsoleMVC::View::ConfirmView::Render(summary, prompt);
        }

        static void PrintDecisionConfirmation(
            const Model::Order& order,
            const std::string& prompt = "Approve this order? (Y/N) > ")
        {
            std::cout << RenderDecisionConfirmation(order, prompt);
        }

    private:
        // Maps each example-domain OrderState to a generic BadgeStyle: the
        // terminal failure state is an Error, the in-flight production
        // state is a Warning, and the remaining (accepted/stable) states
        // are Normal.
        static std::string StatusBadge(Model::OrderState state)
        {
            switch (state)
            {
            case Model::OrderState::PRODUCING:
                return ConsoleMVC::View::BadgeView::Render("PRODUCING", ConsoleMVC::View::BadgeStyle::Warning);
            case Model::OrderState::REJECTED:
                return ConsoleMVC::View::BadgeView::Render("REJECTED", ConsoleMVC::View::BadgeStyle::Error);
            case Model::OrderState::RESERVED:
                return ConsoleMVC::View::BadgeView::Render("RESERVED", ConsoleMVC::View::BadgeStyle::Normal);
            case Model::OrderState::CONFIRMED:
                return ConsoleMVC::View::BadgeView::Render("CONFIRMED", ConsoleMVC::View::BadgeStyle::Normal);
            case Model::OrderState::RELEASE:
            default:
                return ConsoleMVC::View::BadgeView::Render("RELEASE", ConsoleMVC::View::BadgeStyle::Normal);
            }
        }

        static std::vector<std::vector<std::string>> ToRows(const std::vector<Model::Order>& orders)
        {
            std::vector<std::vector<std::string>> rows;
            rows.reserve(orders.size());
            for (const auto& order : orders)
            {
                rows.push_back({
                    std::to_string(order.GetId()),
                    std::to_string(order.GetSampleId()),
                    order.GetCustomerName(),
                    std::to_string(order.GetOrderedQuantity()),
                    StatusBadge(order.GetState())
                });
            }
            return rows;
        }
    };
}
