#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "모니터링").
//
// Builds the read-only "모니터링" screen: Example::View::MonitoringView needs
// every Sample plus, per sample, the total outstanding (not yet shipped)
// order quantity - this factory is the piece that aggregates
// ExampleAppContext::orderModel into the std::map<int,int> MonitoringView
// expects. "Outstanding" is every order that has not yet reached a terminal
// state (RELEASE/REJECTED) - i.e. RESERVED, CONFIRMED, or PRODUCING - since
// all three still represent demand the sample's stock is committed to.

#include "ExampleAppContext.h"
#include "../Model/Order.h"
#include "../View/MonitoringView.h"
#include "../../Controller/MenuNode.h"

#include <chrono>
#include <ctime>
#include <map>
#include <memory>
#include <string>

namespace ConsoleMVC::Example::Controller
{
    class MonitoringMenuFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> Build(ExampleAppContext& context)
        {
            return std::make_shared<ConsoleMVC::Controller::ReadOnlyMenuNode>(
                "Monitoring",
                [&context] {
                    View::MonitoringView::Print(
                        CurrentTimestamp(), context.sampleModel.GetAll(), OutstandingQuantityBySampleId(context));
                });
        }

    private:
        static bool IsOutstanding(Model::OrderState state)
        {
            return state == Model::OrderState::RESERVED
                || state == Model::OrderState::CONFIRMED
                || state == Model::OrderState::PRODUCING;
        }

        static std::map<int, int> OutstandingQuantityBySampleId(const ExampleAppContext& context)
        {
            std::map<int, int> totals;
            for (const Model::Order& order : context.orderModel.GetAll())
            {
                if (IsOutstanding(order.GetState()))
                {
                    totals[order.GetSampleId()] += order.GetOrderedQuantity();
                }
            }
            return totals;
        }

        static std::string CurrentTimestamp()
        {
            const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm localTime{};
            localtime_s(&localTime, &now);

            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
            return std::string(buffer);
        }
    };
}
