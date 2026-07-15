#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/model.md section 6.4 - "주문 승인/생산 배정 도메인 규칙").
//
// ProductionOrchestrator owns the two pieces of combination logic that
// OrderApprovalService deliberately leaves out:
//
//   - TryStartNextWaitingEntry: promotes the production queue's front entry
//     from WAITING to PRODUCING, but only when no entry is currently
//     PRODUCING. Because the queue is a single global FIFO (there is no
//     per-sample queue) and entries are only ever promoted from the front,
//     "no entry is currently PRODUCING" reduces to "the front entry is not
//     already PRODUCING" - this is what enforces
//     docs/feature/model.md section 6.4 rule 3 (one production line for the
//     whole system).
//   - CompleteFrontEntry: finishes the front entry (must be PRODUCING),
//     applying its actual production quantity to the covering Sample's
//     stock (rule 1/2 - no re-applying yield) and reserving exactly the
//     shortage portion for the covering Order (so that, by the time the
//     order reaches CONFIRMED, the sample has the *entire* ordered quantity
//     reserved for it - see the comment on ApproveWithProduction in
//     OrderApprovalService.h), transitions the covering Order to CONFIRMED,
//     removes the entry from the queue, and then tries to start the next
//     WAITING entry.
//
// Core Model/FifoQueue.h only exposes PeekNext() (front, by value) and
// DequeueCompleted() (pop front) - it has no "replace/update the front
// entry in place" operation, and this file must not modify the core. Since
// ProductionQueueEntry is a plain value type, TryStartNextWaitingEntry
// works around this by draining the queue into a temporary buffer,
// re-enqueuing the (now PRODUCING) front entry first, and then the
// remaining entries in their original order - the queue's core Enqueue/
// PeekNext/DequeueCompleted contract is used exactly as published.

#include "Order.h"
#include "ProductionQueueEntry.h"
#include "Sample.h"
#include "ServiceOutcome.h"
#include "../../Model/FifoQueue.h"
#include "../../Model/InMemoryModel.h"

#include <optional>
#include <vector>

namespace ConsoleMVC::Example::Model
{
    class ProductionOrchestrator
    {
    public:
        // Promotes the queue's front entry to PRODUCING if it is currently
        // WAITING. No-op (returns false) if the queue is empty or its front
        // entry is not WAITING (in particular, if it is already PRODUCING -
        // this is exactly the check that keeps the production line to a
        // single active entry system-wide).
        static bool TryStartNextWaitingEntry(ConsoleMVC::Model::FifoQueue<ProductionQueueEntry>& queue)
        {
            const std::optional<ProductionQueueEntry> front = queue.PeekNext();
            if (!front || front->GetState() != ProductionState::WAITING)
            {
                return false;
            }

            ProductionQueueEntry started = *front;
            if (!started.TryTransitionTo(ProductionState::PRODUCING))
            {
                return false;
            }

            ReplaceFrontEntry(queue, started);
            return true;
        }

        // Completes the queue's front entry: it must currently be
        // PRODUCING. On success: the covering Sample gains stock equal to
        // the entry's actual production quantity and reserves the entry's
        // shortage quantity out of that new stock for the covering Order;
        // the covering Order transitions PRODUCING -> CONFIRMED; the entry
        // is removed from the queue; and the next WAITING entry (if any) is
        // started. On failure, nothing is changed - neither model, nor the
        // queue.
        static ServiceOutcome CompleteFrontEntry(
            ConsoleMVC::Model::FifoQueue<ProductionQueueEntry>& queue,
            ConsoleMVC::Model::InMemoryModel<Sample>& sampleModel,
            ConsoleMVC::Model::InMemoryModel<Order>& orderModel)
        {
            const std::optional<ProductionQueueEntry> front = queue.PeekNext();
            if (!front || front->GetState() != ProductionState::PRODUCING)
            {
                return {false, "No production entry is currently in progress."};
            }

            const std::optional<Sample> sample = sampleModel.GetById(front->GetSampleId());
            const std::optional<Order> order = orderModel.GetById(front->GetId());
            if (!sample || !order)
            {
                return {false, "The sample or order covered by this production entry no longer exists."};
            }

            Sample updatedSample = *sample;
            if (!updatedSample.TryIncreaseStock(front->GetActualProductionQuantity()))
            {
                return {false, "Failed to add the produced quantity to stock."};
            }

            const int shortageQuantity = front->GetShortageQuantity();
            if (shortageQuantity > 0 && !updatedSample.TryReserve(shortageQuantity))
            {
                return {false, "Failed to reserve the produced shortage for its covering order."};
            }

            Order updatedOrder = *order;
            if (!updatedOrder.TryTransitionTo(OrderState::CONFIRMED))
            {
                return {false, "Order state transition to CONFIRMED was rejected."};
            }

            ProductionQueueEntry completedEntry = *front;
            if (!completedEntry.TryTransitionTo(ProductionState::CONFIRMED))
            {
                return {false, "Production entry state transition to CONFIRMED was rejected."};
            }

            sampleModel.Update(updatedSample);
            orderModel.Update(updatedOrder);
            queue.DequeueCompleted();
            TryStartNextWaitingEntry(queue);

            return {true, "Production completed; stock and order updated."};
        }

    private:
        // Rebuilds `queue` with `newFront` as its first element followed by
        // the remaining entries in their original order (see the file
        // comment above for why this drain-and-rebuild is necessary).
        static void ReplaceFrontEntry(
            ConsoleMVC::Model::FifoQueue<ProductionQueueEntry>& queue,
            const ProductionQueueEntry& newFront)
        {
            queue.DequeueCompleted();

            std::vector<ProductionQueueEntry> remainingEntries;
            while (const std::optional<ProductionQueueEntry> next = queue.PeekNext())
            {
                remainingEntries.push_back(*next);
                queue.DequeueCompleted();
            }

            queue.Enqueue(newFront);
            for (const ProductionQueueEntry& entry : remainingEntries)
            {
                queue.Enqueue(entry);
            }
        }
    };
}
