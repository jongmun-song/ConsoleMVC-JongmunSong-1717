#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "출고 처리").
//
// Builds the "출고 처리" ActionMenuNode: releases a CONFIRMED order
// (Example::Model::OrderApprovalService::Release) after a Y/N confirmation
// (Example::View::ShipmentView), following the same "collect input, then
// confirm" pattern as SampleMenuFactory/OrderMenuFactory (see those files'
// comments for why ActionMenuNode's buildSummary constructor is not used
// here either).

#include "ConsoleInputHelpers.h"
#include "ExampleAppContext.h"
#include "../Model/Order.h"
#include "../Model/OrderApprovalService.h"
#include "../Model/Sample.h"
#include "../View/ShipmentView.h"
#include "../../Controller/MenuNode.h"

#include <memory>
#include <optional>

namespace ConsoleMVC::Example::Controller
{
    class ShipmentMenuFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> Build(ExampleAppContext& context)
        {
            return std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Release a shipment",
                [&context] { return Release(context); });
        }

    private:
        static ConsoleMVC::Controller::ActionOutcome Release(ExampleAppContext& context)
        {
            const int orderId = ConsoleInputHelpers::ReadInt("Order ID to release (0 to cancel) > ");
            if (orderId == 0)
            {
                return {true, "Cancelled."};
            }

            std::optional<Model::Order> orderLookup = context.orderModel.GetById(orderId);
            if (!orderLookup || orderLookup->GetState() != Model::OrderState::CONFIRMED)
            {
                return {false, "No confirmed order exists with that ID."};
            }

            std::optional<Model::Sample> sampleLookup = context.sampleModel.GetById(orderLookup->GetSampleId());
            if (!sampleLookup)
            {
                return {false, "The sample covering this order no longer exists."};
            }

            Model::Order order = *orderLookup;
            Model::Sample sample = *sampleLookup;

            View::ShipmentView::PrintConfirmation(order.GetId(), sample.GetName(), order.GetOrderedQuantity());
            if (!ConsoleInputHelpers::ReadYesNo())
            {
                return {true, "Cancelled."};
            }

            const Model::ServiceOutcome result = Model::OrderApprovalService::Release(order, sample);
            if (result.succeeded)
            {
                context.orderModel.Update(order);
                context.sampleModel.Update(sample);
            }
            return {result.succeeded, result.message};
        }
    };
}
