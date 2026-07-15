#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "시료 주문" / "주문 승인/거절").
//
// Builds two pieces of the example menu tree:
//   - BuildOrderIntakeAction: a single ActionMenuNode that places a new
//     RESERVED order (Example::View::OrderIntakeView).
//   - BuildApprovalMenu: a submenu with "Approve" / "Reject" ActionMenuNodes
//     that apply docs/feature/model.md section 6.4's confirmed domain rules
//     via Example::Model::OrderApprovalService (Approve()/Reject()).
//
// As with SampleMenuFactory, these handlers build their own Y/N
// confirmation (via ConsoleInputHelpers, on top of the same
// Controller::InputReader parser ActionMenuNode's built-in confirmation
// uses) rather than ActionMenuNode's buildSummary+confirm constructor,
// because the confirmation summary depends on input (a sample id / an order
// id) collected at handler run time. Answering N always returns before any
// Model is touched.

#include "ConsoleInputHelpers.h"
#include "ExampleAppContext.h"
#include "../Model/Order.h"
#include "../Model/OrderApprovalService.h"
#include "../Model/Sample.h"
#include "../View/OrderApprovalView.h"
#include "../View/OrderIntakeView.h"
#include "../../Controller/MenuNode.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ConsoleMVC::Example::Controller
{
    class OrderMenuFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> BuildIntakeAction(ExampleAppContext& context)
        {
            return std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Place a new order",
                [&context] { return PlaceOrder(context); });
        }

        static std::shared_ptr<ConsoleMVC::Controller::IController> BuildApprovalMenu(ExampleAppContext& context)
        {
            auto menu = std::make_shared<ConsoleMVC::Controller::MenuNode>("Order Approval / Rejection");

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Approve a reserved order",
                [&context] { return DecideOrder(context, true); }));

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Reject a reserved order",
                [&context] { return DecideOrder(context, false); }));

            return menu;
        }

    private:
        static ConsoleMVC::Controller::ActionOutcome PlaceOrder(ExampleAppContext& context)
        {
            const int sampleId = ConsoleInputHelpers::ReadInt("Sample ID > ");
            if (!context.sampleModel.GetById(sampleId).has_value())
            {
                return {false, "No sample exists with that ID."};
            }

            const std::string customerName = ConsoleInputHelpers::ReadNonEmptyLine("Customer name > ");
            const int orderedQuantity = ConsoleInputHelpers::ReadInt("Ordered quantity > ");
            if (orderedQuantity <= 0)
            {
                return {false, "Ordered quantity must be positive."};
            }

            View::OrderIntakeView::PrintConfirmation(sampleId, customerName, orderedQuantity);
            if (!ConsoleInputHelpers::ReadYesNo())
            {
                return {true, "Cancelled."};
            }

            context.orderModel.Add(Model::Order(context.nextOrderId, sampleId, customerName, orderedQuantity));
            context.TakeNextOrderId();
            return {true, "Order placed (RESERVED)."};
        }

        // `approve` selects between the "Approve" and "Reject" flows, which
        // otherwise share every step except which OrderApprovalService call
        // they make and which confirmation prompt they show.
        static ConsoleMVC::Controller::ActionOutcome DecideOrder(ExampleAppContext& context, bool approve)
        {
            const std::vector<Model::Order> pendingOrders = context.orderModel.Find(
                [](const Model::Order& order) { return order.GetState() == Model::OrderState::RESERVED; });

            View::OrderApprovalView::PrintPendingOrders(pendingOrders);
            if (pendingOrders.empty())
            {
                return {true, "No reserved orders to decide on."};
            }

            const int orderId = ConsoleInputHelpers::ReadInt("Order ID to decide on (0 to cancel) > ");
            if (orderId == 0)
            {
                return {true, "Cancelled."};
            }

            std::optional<Model::Order> orderLookup = context.orderModel.GetById(orderId);
            if (!orderLookup || orderLookup->GetState() != Model::OrderState::RESERVED)
            {
                return {false, "No reserved order exists with that ID."};
            }

            Model::Order order = *orderLookup;
            View::OrderApprovalView::PrintDecisionConfirmation(
                order, approve ? "Approve this order? (Y/N) > " : "Reject this order? (Y/N) > ");
            if (!ConsoleInputHelpers::ReadYesNo())
            {
                return {true, "Cancelled."};
            }

            if (!approve)
            {
                const Model::ServiceOutcome result = Model::OrderApprovalService::Reject(order);
                if (result.succeeded)
                {
                    context.orderModel.Update(order);
                }
                return {result.succeeded, result.message};
            }

            std::optional<Model::Sample> sampleLookup = context.sampleModel.GetById(order.GetSampleId());
            if (!sampleLookup)
            {
                return {false, "The sample covering this order no longer exists."};
            }

            Model::Sample sample = *sampleLookup;
            const Model::ServiceOutcome result =
                Model::OrderApprovalService::Approve(order, sample, context.productionQueue);
            if (result.succeeded)
            {
                context.orderModel.Update(order);
                context.sampleModel.Update(sample);
            }
            return {result.succeeded, result.message};
        }
    };
}
