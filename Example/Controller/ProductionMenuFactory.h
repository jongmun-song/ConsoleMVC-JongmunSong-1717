#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "생산 라인 현황").
//
// Builds the "생산 라인" submenu: a ReadOnlyMenuNode showing the current
// queue (Example::View::ProductionLineView) and an ActionMenuNode,
// "Advance production line", that drives Example::Model::
// ProductionOrchestrator - the only way any ProductionQueueEntry ever
// changes state in this demo.
//
// Deviation from docs/feature/controller.md section 7's example tree (see
// the final report for this task): that section's tree only lists a
// read-only "생산 라인 현황" item. This demo has no real clock/timer driving
// production to completion in the background, so without an explicit
// action a WAITING entry could never become PRODUCING, and a PRODUCING
// entry could never complete (which would also mean its covering order
// could never reach CONFIRMED, and shipment/release could never be
// exercised). Adding this action is necessary for the queue's WAITING ->
// PRODUCING -> CONFIRMED lifecycle (docs/feature/model.md section 6.3) and
// the single-production-line invariant (section 6.4 rule 3) to be
// observable at all in an interactive demo; it does not change any Model
// contract and the section itself says the example tree is
// "참고용, 고정 사양 아님" (reference only, not a fixed spec).
//
// One "Advance" press does exactly one of the following, whichever applies:
//   - if the front entry is WAITING, start it (-> PRODUCING);
//   - else if the front entry is PRODUCING, complete it (-> CONFIRMED,
//     apply stock/order effects, dequeue it, and try to start the next
//     WAITING entry).
// This has no destructive-looking prompt (unlike order approval/rejection
// or shipment) - it only ever moves the single global queue forward - so it
// runs without an extra confirmation step, consistent with
// docs/feature/controller.md section 3.2 ("어떤 액션이 '상태 변경 액션'에
// 해당하는지는 도메인 계층이 결정한다").

#include "ExampleAppContext.h"
#include "../Model/ProductionOrchestrator.h"
#include "../Model/ProductionQueueEntry.h"
#include "../View/ProductionLineView.h"
#include "../../Controller/MenuNode.h"

#include <deque>
#include <memory>
#include <optional>
#include <vector>

namespace ConsoleMVC::Example::Controller
{
    class ProductionMenuFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> Build(ExampleAppContext& context)
        {
            auto menu = std::make_shared<ConsoleMVC::Controller::MenuNode>("Production Line");

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ReadOnlyMenuNode>(
                "Show production queue",
                [&context] { View::ProductionLineView::Print(SnapshotQueue(context)); }));

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Advance production line",
                [&context] { return Advance(context); }));

            return menu;
        }

    private:
        // FifoQueue<T> only exposes PeekNext()/DequeueCompleted() (front-only
        // access - see Model/FifoQueue.h) rather than iteration, so a
        // display snapshot is built the same drain-and-rebuild way
        // Example::Model::ProductionOrchestrator does internally: nothing
        // here is removed permanently, only copied out into a vector for
        // View::ProductionLineView::Print, then re-enqueued in the same
        // order.
        static std::vector<Model::ProductionQueueEntry> SnapshotQueue(ExampleAppContext& context)
        {
            std::vector<Model::ProductionQueueEntry> entries;
            while (const std::optional<Model::ProductionQueueEntry> next = context.productionQueue.PeekNext())
            {
                entries.push_back(*next);
                context.productionQueue.DequeueCompleted();
            }
            for (const Model::ProductionQueueEntry& entry : entries)
            {
                context.productionQueue.Enqueue(entry);
            }
            return entries;
        }

        static ConsoleMVC::Controller::ActionOutcome Advance(ExampleAppContext& context)
        {
            const std::optional<Model::ProductionQueueEntry> front = context.productionQueue.PeekNext();
            if (!front)
            {
                return {true, "Production queue is empty."};
            }

            if (front->GetState() == Model::ProductionState::WAITING)
            {
                const bool started = Model::ProductionOrchestrator::TryStartNextWaitingEntry(context.productionQueue);
                return {started, started ? "Started production for the next queued entry." : "Could not start production."};
            }

            if (front->GetState() == Model::ProductionState::PRODUCING)
            {
                const Model::ServiceOutcome result = Model::ProductionOrchestrator::CompleteFrontEntry(
                    context.productionQueue, context.sampleModel, context.orderModel);
                return {result.succeeded, result.message};
            }

            return {true, "Nothing to advance."};
        }
    };
}
