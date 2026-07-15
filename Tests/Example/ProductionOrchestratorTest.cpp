// Example/Model/ProductionOrchestrator.h contract tests (PoC verification
// domain, Phase 3-B).
//
// docs/feature/model.md section 6.4:
//   rule 3 - "생산 라인은 시스템 전체에 1개뿐이다": at most one
//            ProductionQueueEntry may be PRODUCING at a time, system-wide.
//   rule 1/2 - the stock increase on completion is exactly
//              GetActualProductionQuantity() (ceil(shortage / yield)), with
//              no re-application of the yield ratio.

#include <gtest/gtest.h>

#include "../../Example/Model/Order.h"
#include "../../Example/Model/OrderApprovalService.h"
#include "../../Example/Model/ProductionOrchestrator.h"
#include "../../Example/Model/ProductionQueueEntry.h"
#include "../../Example/Model/Sample.h"
#include "../../Model/FifoQueue.h"
#include "../../Model/InMemoryModel.h"

namespace
{
    using ConsoleMVC::Example::Model::Order;
    using ConsoleMVC::Example::Model::OrderApprovalService;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Example::Model::ProductionOrchestrator;
    using ConsoleMVC::Example::Model::ProductionQueueEntry;
    using ConsoleMVC::Example::Model::ProductionState;
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Model::FifoQueue;
    using ConsoleMVC::Model::InMemoryModel;

    // Builds a WAITING ProductionQueueEntry for `order` against `sample`, via
    // the same path production entries are actually created in the app
    // (OrderApprovalService::Approve with a shortage), and returns after
    // adding both entities to the given repositories.
    void ApproveWithShortageAndStore(
        Order order,
        Sample sample,
        FifoQueue<ProductionQueueEntry>& queue,
        InMemoryModel<Sample>& sampleModel,
        InMemoryModel<Order>& orderModel)
    {
        ASSERT_TRUE(OrderApprovalService::Approve(order, sample, queue).succeeded);
        ASSERT_EQ(order.GetState(), OrderState::PRODUCING);
        sampleModel.Add(sample);
        orderModel.Add(order);
    }
}

TEST(ProductionOrchestratorTest, TryStartNextWaitingEntryPromotesOnlyTheFrontEntry)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    Sample sample(1, "Widget", 1.0, 0.5, 0);
    ApproveWithShortageAndStore(Order(1, 1, "Customer A", 10), sample, queue, sampleModel, orderModel);
    ApproveWithShortageAndStore(Order(2, 1, "Customer B", 10), sample, queue, sampleModel, orderModel);
    ASSERT_EQ(queue.Size(), 2u);

    const bool started = ProductionOrchestrator::TryStartNextWaitingEntry(queue);

    EXPECT_TRUE(started);
    ASSERT_EQ(queue.Size(), 2u);
    const auto front = queue.PeekNext();
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(front->GetId(), 1); // first order's entry
    EXPECT_EQ(front->GetState(), ProductionState::PRODUCING);
}

TEST(ProductionOrchestratorTest, TryStartNextWaitingEntryIsNoOpWhenFrontAlreadyProducing)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    Sample sample(1, "Widget", 1.0, 0.5, 0);
    ApproveWithShortageAndStore(Order(1, 1, "Customer A", 10), sample, queue, sampleModel, orderModel);
    ApproveWithShortageAndStore(Order(2, 1, "Customer B", 10), sample, queue, sampleModel, orderModel);
    ASSERT_TRUE(ProductionOrchestrator::TryStartNextWaitingEntry(queue));

    // Second call: front is already PRODUCING - rule 3 forbids a second
    // concurrently-PRODUCING entry, so this must do nothing at all, and in
    // particular must not promote the second (still WAITING) entry.
    const bool startedAgain = ProductionOrchestrator::TryStartNextWaitingEntry(queue);

    EXPECT_FALSE(startedAgain);
    ASSERT_EQ(queue.Size(), 2u);
    const auto front = queue.PeekNext();
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(front->GetId(), 1);
    EXPECT_EQ(front->GetState(), ProductionState::PRODUCING);

    // Drain to confirm the second entry is still WAITING (never promoted).
    queue.DequeueCompleted();
    const auto second = queue.PeekNext();
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(second->GetId(), 2);
    EXPECT_EQ(second->GetState(), ProductionState::WAITING);
}

TEST(ProductionOrchestratorTest, CompleteFrontEntryAppliesExactActualProductionQuantityToStockNoDoubleYield)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    // shortage 10, yield 0.5 -> actual production quantity = ceil(10/0.5) = 20.
    Sample sample(1, "Widget", 1.0, 0.5, 0);
    ApproveWithShortageAndStore(Order(1, 1, "Customer A", 10), sample, queue, sampleModel, orderModel);
    ASSERT_TRUE(ProductionOrchestrator::TryStartNextWaitingEntry(queue));

    const auto beforeCompletion = queue.PeekNext();
    ASSERT_TRUE(beforeCompletion.has_value());
    ASSERT_EQ(beforeCompletion->GetActualProductionQuantity(), 20);

    const auto result = ProductionOrchestrator::CompleteFrontEntry(queue, sampleModel, orderModel);

    EXPECT_TRUE(result.succeeded);

    const auto updatedSample = sampleModel.GetById(1);
    ASSERT_TRUE(updatedSample.has_value());
    // rule 1/2: stock increases by exactly the actual production quantity
    // (20), not e.g. 20 * 0.5 = 10 (which would be re-applying yield).
    EXPECT_EQ(updatedSample->GetStockQuantity(), 20);

    const auto updatedOrder = orderModel.GetById(1);
    ASSERT_TRUE(updatedOrder.has_value());
    EXPECT_EQ(updatedOrder->GetState(), OrderState::CONFIRMED);

    EXPECT_TRUE(queue.IsEmpty()); // the completed entry was removed
}

TEST(ProductionOrchestratorTest, CompleteFrontEntryAutoPromotesTheNextWaitingEntry)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    Sample sample(1, "Widget", 1.0, 0.5, 0);
    ApproveWithShortageAndStore(Order(1, 1, "Customer A", 10), sample, queue, sampleModel, orderModel);
    ApproveWithShortageAndStore(Order(2, 1, "Customer B", 10), sample, queue, sampleModel, orderModel);
    ASSERT_TRUE(ProductionOrchestrator::TryStartNextWaitingEntry(queue));
    ASSERT_EQ(queue.Size(), 2u);

    const auto result = ProductionOrchestrator::CompleteFrontEntry(queue, sampleModel, orderModel);

    ASSERT_TRUE(result.succeeded);
    ASSERT_EQ(queue.Size(), 1u); // first entry removed, second remains

    const auto front = queue.PeekNext();
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(front->GetId(), 2);
    // rule 3: the queue must always have at most one PRODUCING entry, so
    // completing the first must automatically promote the next WAITING one.
    EXPECT_EQ(front->GetState(), ProductionState::PRODUCING);
}

TEST(ProductionOrchestratorTest, CompleteFrontEntryFailsWhenFrontIsStillWaiting)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    Sample sample(1, "Widget", 1.0, 0.5, 0);
    ApproveWithShortageAndStore(Order(1, 1, "Customer A", 10), sample, queue, sampleModel, orderModel);
    // Deliberately do not call TryStartNextWaitingEntry - front stays WAITING.

    const auto result = ProductionOrchestrator::CompleteFrontEntry(queue, sampleModel, orderModel);

    EXPECT_FALSE(result.succeeded);
    ASSERT_EQ(queue.Size(), 1u);
    const auto front = queue.PeekNext();
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(front->GetState(), ProductionState::WAITING);

    const auto unchangedSample = sampleModel.GetById(1);
    ASSERT_TRUE(unchangedSample.has_value());
    EXPECT_EQ(unchangedSample->GetStockQuantity(), 0);

    const auto unchangedOrder = orderModel.GetById(1);
    ASSERT_TRUE(unchangedOrder.has_value());
    EXPECT_EQ(unchangedOrder->GetState(), OrderState::PRODUCING);
}

TEST(ProductionOrchestratorTest, CompleteFrontEntryFailsWhenQueueIsEmpty)
{
    FifoQueue<ProductionQueueEntry> queue;
    InMemoryModel<Sample> sampleModel;
    InMemoryModel<Order> orderModel;

    const auto result = ProductionOrchestrator::CompleteFrontEntry(queue, sampleModel, orderModel);

    EXPECT_FALSE(result.succeeded);
    EXPECT_TRUE(queue.IsEmpty());
}
