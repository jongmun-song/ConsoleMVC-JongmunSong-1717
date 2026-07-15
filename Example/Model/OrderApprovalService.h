#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/model.md section 6.4 - "주문 승인/생산 배정 도메인 규칙").
//
// OrderApprovalService is the combination logic that ties Order, Sample and
// the production queue together for the RESERVED -> {CONFIRMED, PRODUCING,
// REJECTED} transition and the later CONFIRMED -> RELEASE (shipment)
// transition. It performs no console I/O and reads/writes only the plain
// Order/Sample values it is given - Example/Controller is responsible for
// loading them from (and writing them back to) the InMemoryModel<T>
// repositories and for any user-facing confirmation prompts.
//
// This file intentionally does not touch Example/Model/Order.h,
// Example/Model/Sample.h's existing members, or Example/Model/
// ProductionQueueEntry.h - it only calls the public contracts those types
// already expose (plus the reservation members added to Sample.h for this
// task).
//
// Rules implemented here (docs/feature/model.md section 6.4):
//   1/2. Production quantity is exactly ProductionQueueEntry's own
//        CalculateActualProductionQuantity(shortage, yield) - computed once
//        inside the ProductionQueueEntry constructor - and is applied to
//        stock without re-applying the yield ratio a second time.
//   3.   The single global production line is enforced by
//        ProductionOrchestrator.h, not here - Approve() only ever enqueues a
//        new entry as WAITING.
//   4.   Stock is never decremented at approval time; only Sample's reserved
//        quantity changes (via TryReserve).
//   5.   The shortage is always computed from the sample's *current*
//        available quantity (GetAvailableQuantity()), never from a
//        projection of what other queued/producing entries might yield.

#include "Order.h"
#include "ProductionQueueEntry.h"
#include "Sample.h"
#include "ServiceOutcome.h"
#include "../../Model/FifoQueue.h"

namespace ConsoleMVC::Example::Model
{
    class OrderApprovalService
    {
    public:
        // Approves a RESERVED order against `sample` (its covering sample -
        // the caller is responsible for matching sample.GetId() ==
        // order.GetSampleId()). On success, `order` and `sample` are updated
        // in place and, if the available stock could not fully cover the
        // order, a new WAITING ProductionQueueEntry for the shortage is
        // appended to `productionQueue`. On failure, neither `order` nor
        // `sample` nor `productionQueue` is changed.
        static ServiceOutcome Approve(
            Order& order,
            Sample& sample,
            ConsoleMVC::Model::FifoQueue<ProductionQueueEntry>& productionQueue)
        {
            if (order.GetState() != OrderState::RESERVED)
            {
                return {false, "Only a reserved order can be approved."};
            }

            const int orderedQuantity = order.GetOrderedQuantity();
            const int availableQuantity = sample.GetAvailableQuantity();

            if (availableQuantity >= orderedQuantity)
            {
                return ApproveFromAvailableStock(order, sample, orderedQuantity);
            }

            return ApproveWithProduction(order, sample, productionQueue, orderedQuantity, availableQuantity);
        }

        // Rejects a RESERVED order. Only a RESERVED order can be rejected
        // (requirements.pdf Chapter 2 / Order.h's own transition table) -
        // once an order has moved on to CONFIRMED/PRODUCING, rejection is no
        // longer a valid outcome and this call fails without changing
        // `order`.
        static ServiceOutcome Reject(Order& order)
        {
            if (!order.TryTransitionTo(OrderState::REJECTED))
            {
                return {false, "Only a reserved order can be rejected."};
            }
            return {true, "Order rejected."};
        }

        // Releases (ships) a CONFIRMED order: fulfils its reservation on
        // `sample` (decreasing both reserved and actual stock by the full
        // ordered quantity) and transitions `order` to RELEASE. On failure,
        // neither `order` nor `sample` is changed.
        static ServiceOutcome Release(Order& order, Sample& sample)
        {
            if (order.GetState() != OrderState::CONFIRMED)
            {
                return {false, "Only a confirmed order can be released."};
            }

            const int orderedQuantity = order.GetOrderedQuantity();
            if (!sample.TryFulfillReservation(orderedQuantity))
            {
                return {false, "Reserved/actual stock is insufficient to release this order."};
            }

            if (!order.TryTransitionTo(OrderState::RELEASE))
            {
                // Roll back the reservation fulfilment so a rejected
                // transition leaves state entirely unchanged.
                sample.TryReserve(orderedQuantity);
                sample.TryIncreaseStock(orderedQuantity);
                return {false, "Order state transition to RELEASE was rejected."};
            }

            return {true, "Order released for shipment."};
        }

    private:
        static ServiceOutcome ApproveFromAvailableStock(Order& order, Sample& sample, int orderedQuantity)
        {
            if (!sample.TryReserve(orderedQuantity))
            {
                return {false, "Failed to reserve stock for this order."};
            }

            if (!order.TryTransitionTo(OrderState::CONFIRMED))
            {
                sample.TryReleaseReservation(orderedQuantity);
                return {false, "Order state transition to CONFIRMED was rejected."};
            }

            return {true, "Order confirmed directly from available stock."};
        }

        static ServiceOutcome ApproveWithProduction(
            Order& order,
            Sample& sample,
            ConsoleMVC::Model::FifoQueue<ProductionQueueEntry>& productionQueue,
            int orderedQuantity,
            int availableQuantity)
        {
            if (availableQuantity > 0 && !sample.TryReserve(availableQuantity))
            {
                return {false, "Failed to reserve available stock for this order."};
            }

            if (!order.TryTransitionTo(OrderState::PRODUCING))
            {
                if (availableQuantity > 0)
                {
                    sample.TryReleaseReservation(availableQuantity);
                }
                return {false, "Order state transition to PRODUCING was rejected."};
            }

            const int shortageQuantity = orderedQuantity - availableQuantity;
            productionQueue.Enqueue(ProductionQueueEntry(
                order.GetId(),
                sample.GetId(),
                orderedQuantity,
                shortageQuantity,
                sample.GetYieldRatio(),
                sample.GetAverageProductionTimePerUnit()));

            return {true, "Insufficient stock; shortage queued for production."};
        }
    };
}
