// Example/Model/OrderApprovalService.h contract tests (PoC verification
// domain, Phase 3-B).
//
// docs/feature/model.md section 6.4 - "주문 승인/생산 배정 도메인 규칙":
//   rule 1/2 - production quantity is ceil(shortage / yield), applied to
//              stock without a second yield application (verified together
//              with rule 3 in ProductionOrchestratorTest.cpp - this file
//              only checks that Approve() enqueues a WAITING entry whose
//              own GetActualProductionQuantity() already reflects rule 1).
//   rule 3   - Approve() never marks the new queue entry PRODUCING itself;
//              that is ProductionOrchestrator's job (see
//              ProductionOrchestratorTest.cpp for the single-production-line
//              invariant).
//   rule 4   - stock is never decremented at approval time; only the
//              reserved quantity changes.
//   rule 5   - shortage is computed only from Sample::GetAvailableQuantity()
//              at decision time, never a projection of other queued/
//              producing entries completing.

#include <gtest/gtest.h>

#include "../../Example/Model/Order.h"
#include "../../Example/Model/OrderApprovalService.h"
#include "../../Example/Model/ProductionQueueEntry.h"
#include "../../Example/Model/Sample.h"
#include "../../Model/FifoQueue.h"

namespace
{
    using ConsoleMVC::Example::Model::Order;
    using ConsoleMVC::Example::Model::OrderApprovalService;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Example::Model::ProductionQueueEntry;
    using ConsoleMVC::Example::Model::ProductionState;
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Model::FifoQueue;
}

TEST(OrderApprovalServiceTest, ApproveFromSufficientStockConfirmsAndReservesOrderedQuantity)
{
    Order order(1, /*sampleId*/ 1, "Customer A", 30);
    Sample sample(1, "Widget", 1.0, 1.0, 100);
    FifoQueue<ProductionQueueEntry> queue;

    const auto result = OrderApprovalService::Approve(order, sample, queue);

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::CONFIRMED);
    EXPECT_EQ(sample.GetReservedQuantity(), 30);
    EXPECT_EQ(sample.GetStockQuantity(), 100); // rule 4: actual stock untouched
    EXPECT_TRUE(queue.IsEmpty());
}

TEST(OrderApprovalServiceTest, ApproveWithShortageMovesToProducingReservesAvailableAndQueuesShortage)
{
    // Available stock (20) covers only part of the order (50) -> shortage 30.
    Order order(1, 1, "Customer A", 50);
    Sample sample(1, "Widget", 1.0, 0.5 /* yield */, 20);
    FifoQueue<ProductionQueueEntry> queue;

    const auto result = OrderApprovalService::Approve(order, sample, queue);

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::PRODUCING);
    EXPECT_EQ(sample.GetReservedQuantity(), 20); // all available stock reserved
    EXPECT_EQ(sample.GetAvailableQuantity(), 0);
    EXPECT_EQ(sample.GetStockQuantity(), 20); // rule 4: stock itself untouched

    ASSERT_FALSE(queue.IsEmpty());
    const auto entry = queue.PeekNext();
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->GetState(), ProductionState::WAITING); // rule 3: never auto-started here
    EXPECT_EQ(entry->GetShortageQuantity(), 30);
    // rule 1: production quantity = ceil(shortage / yield) = ceil(30 / 0.5) = 60.
    EXPECT_EQ(entry->GetActualProductionQuantity(), 60);
}

TEST(OrderApprovalServiceTest, ApproveOnNonReservedOrderFailsAndChangesNothing)
{
    Order order(1, 1, "Customer A", 10);
    Sample sample(1, "Widget", 1.0, 1.0, 100);
    FifoQueue<ProductionQueueEntry> queue;
    ASSERT_TRUE(OrderApprovalService::Approve(order, sample, queue).succeeded);
    ASSERT_EQ(order.GetState(), OrderState::CONFIRMED);

    const auto result = OrderApprovalService::Approve(order, sample, queue);

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::CONFIRMED);
    EXPECT_EQ(sample.GetReservedQuantity(), 10);
}

TEST(OrderApprovalServiceTest, RejectTransitionsToRejectedWithNoSampleOrQueueSideEffects)
{
    Order order(1, 1, "Customer A", 10);
    Sample sample(1, "Widget", 1.0, 1.0, 100);
    FifoQueue<ProductionQueueEntry> queue;

    const auto result = OrderApprovalService::Reject(order);

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::REJECTED);
    EXPECT_EQ(sample.GetStockQuantity(), 100);
    EXPECT_EQ(sample.GetReservedQuantity(), 0);
    EXPECT_TRUE(queue.IsEmpty());
}

TEST(OrderApprovalServiceTest, RejectOnNonReservedOrderFailsAndLeavesStateUnchanged)
{
    Order order(1, 1, "Customer A", 10);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::REJECTED));

    const auto result = OrderApprovalService::Reject(order);

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::REJECTED);
}

TEST(OrderApprovalServiceTest, ReleaseFulfillsReservationAndTransitionsToRelease)
{
    Order order(1, 1, "Customer A", 30);
    Sample sample(1, "Widget", 1.0, 1.0, 100);
    FifoQueue<ProductionQueueEntry> queue;
    ASSERT_TRUE(OrderApprovalService::Approve(order, sample, queue).succeeded);
    ASSERT_EQ(order.GetState(), OrderState::CONFIRMED);

    const auto result = OrderApprovalService::Release(order, sample);

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::RELEASE);
    EXPECT_EQ(sample.GetReservedQuantity(), 0);
    EXPECT_EQ(sample.GetStockQuantity(), 70); // 100 - 30, decremented via TryFulfillReservation
}

TEST(OrderApprovalServiceTest, ReleaseOnNonConfirmedOrderFailsAndChangesNothing)
{
    Order order(1, 1, "Customer A", 30);
    Sample sample(1, "Widget", 1.0, 1.0, 100);

    const auto result = OrderApprovalService::Release(order, sample);

    EXPECT_FALSE(result.succeeded);
    EXPECT_EQ(order.GetState(), OrderState::RESERVED);
    EXPECT_EQ(sample.GetStockQuantity(), 100);
    EXPECT_EQ(sample.GetReservedQuantity(), 0);
}

// Rule 5: shortage/availability is decided purely from Sample::
// GetAvailableQuantity() at the moment of this Approve() call - other
// entries already sitting in the production queue (WAITING or PRODUCING) for
// unrelated orders must have zero influence on this computation, even though
// those entries will eventually add stock back once completed.
TEST(OrderApprovalServiceTest, ApproveIgnoresOtherQueuedEntriesAndOnlyUsesCurrentAvailableQuantity)
{
    Sample sample(1, "Widget", 1.0, 0.5, 5); // only 5 units available right now

    FifoQueue<ProductionQueueEntry> queue;
    // Pre-existing WAITING entry for a different order, covering a large
    // shortage - if this were (incorrectly) considered as "future stock",
    // it might make the new order below look coverable when it is not.
    Order otherOrder(1, 1, "Other Customer", 200);
    Sample otherSampleView(1, "Widget", 1.0, 0.5, 5);
    ASSERT_TRUE(OrderApprovalService::Approve(otherOrder, otherSampleView, queue).succeeded);
    ASSERT_EQ(queue.Size(), 1u);
    // Promote the pre-existing entry to PRODUCING too, to prove a PRODUCING
    // entry's eventual stock contribution is also ignored.
    {
        auto front = queue.PeekNext();
        ASSERT_TRUE(front.has_value());
        ProductionQueueEntry started = *front;
        ASSERT_TRUE(started.TryTransitionTo(ProductionState::PRODUCING));
        queue.DequeueCompleted();
        queue.Enqueue(started);
    }

    // Now approve a brand-new order against the *original* sample (whose
    // current available quantity is unaffected by the queue above).
    Order newOrder(2, 1, "New Customer", 8);
    const auto result = OrderApprovalService::Approve(newOrder, sample, queue);

    EXPECT_TRUE(result.succeeded);
    EXPECT_EQ(newOrder.GetState(), OrderState::PRODUCING);
    // available was exactly 5 (current stock, no projection), so shortage is
    // 8 - 5 = 3, not something smaller that would assume the queued 200-unit
    // shortage had already been produced.
    EXPECT_EQ(sample.GetReservedQuantity(), 5);
    ASSERT_EQ(queue.Size(), 2u);
}
