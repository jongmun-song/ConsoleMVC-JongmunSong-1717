// Example/Model/ProductionQueueEntry.h contract tests (PoC verification
// domain).
//
// docs/feature/model.md section 6.3 - production entries are computed via
// free functions (CalculateActualProductionQuantity /
// CalculateTotalProductionTime), track their own PRODUCING->CONFIRMED
// StatefulModel<ProductionState>, and are processed in FIFO order via the
// core ConsoleMVC::Model::FifoQueue<T>.

#include <gtest/gtest.h>

#include "../../Example/Model/ProductionQueueEntry.h"
#include "../../Model/FifoQueue.h"

namespace
{
    using ConsoleMVC::Example::Model::CalculateActualProductionQuantity;
    using ConsoleMVC::Example::Model::CalculateTotalProductionTime;
    using ConsoleMVC::Example::Model::ProductionQueueEntry;
    using ConsoleMVC::Example::Model::ProductionState;
    using ConsoleMVC::Model::FifoQueue;
}

// --- CalculateActualProductionQuantity ---

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityRoundsUpToCoverShortage)
{
    // ceil(100 / 0.9) = ceil(111.11...) = 112
    const int result = CalculateActualProductionQuantity(100, 0.9);

    EXPECT_EQ(result, 112);
}

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityExactDivisionNeedsNoRounding)
{
    // ceil(100 / 0.5) = 200
    const int result = CalculateActualProductionQuantity(100, 0.5);

    EXPECT_EQ(result, 200);
}

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityReturnsZeroForZeroShortage)
{
    EXPECT_EQ(CalculateActualProductionQuantity(0, 0.9), 0);
}

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityReturnsZeroForNegativeShortage)
{
    EXPECT_EQ(CalculateActualProductionQuantity(-5, 0.9), 0);
}

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityReturnsZeroForZeroYield)
{
    EXPECT_EQ(CalculateActualProductionQuantity(100, 0.0), 0);
}

TEST(ProductionQueueEntryTest, CalculateActualProductionQuantityReturnsZeroForNegativeYield)
{
    EXPECT_EQ(CalculateActualProductionQuantity(100, -0.5), 0);
}

// --- CalculateTotalProductionTime ---

TEST(ProductionQueueEntryTest, CalculateTotalProductionTimeMultipliesAvgTimeByQuantity)
{
    const double result = CalculateTotalProductionTime(2.5, 112);

    EXPECT_DOUBLE_EQ(result, 280.0);
}

TEST(ProductionQueueEntryTest, CalculateTotalProductionTimeIsZeroForZeroQuantity)
{
    const double result = CalculateTotalProductionTime(2.5, 0);

    EXPECT_DOUBLE_EQ(result, 0.0);
}

// --- ProductionQueueEntry construction / derived fields ---

TEST(ProductionQueueEntryTest, ConstructorDerivesActualQuantityAndTotalTime)
{
    // shortage=100, yield=0.9 -> actual=112; avgTime=2.0 -> total=224.0
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);

    EXPECT_EQ(entry.GetId(), 1);
    EXPECT_EQ(entry.GetSampleId(), 100);
    EXPECT_EQ(entry.GetOrderedQuantity(), 50);
    EXPECT_EQ(entry.GetShortageQuantity(), 100);
    EXPECT_EQ(entry.GetActualProductionQuantity(), 112);
    EXPECT_DOUBLE_EQ(entry.GetTotalProductionTime(), 224.0);
}

TEST(ProductionQueueEntryTest, NewEntryStartsInProducingState)
{
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);

    EXPECT_EQ(entry.GetState(), ProductionState::PRODUCING);
}

TEST(ProductionQueueEntryTest, ProducingToConfirmedTransitionSucceeds)
{
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);

    const bool result = entry.TryTransitionTo(ProductionState::CONFIRMED);

    EXPECT_TRUE(result);
    EXPECT_EQ(entry.GetState(), ProductionState::CONFIRMED);
}

TEST(ProductionQueueEntryTest, ConfirmedIsTerminalAndRejectsFurtherTransition)
{
    ProductionQueueEntry entry(1, 100, 50, 100, 0.9, 2.0);
    ASSERT_TRUE(entry.TryTransitionTo(ProductionState::CONFIRMED));

    const bool result = entry.TryTransitionTo(ProductionState::PRODUCING);

    EXPECT_FALSE(result);
    EXPECT_EQ(entry.GetState(), ProductionState::CONFIRMED);
}

// --- FifoQueue<ProductionQueueEntry> integration ---

TEST(ProductionQueueEntryTest, FifoQueueProcessesEntriesInEnqueueOrder)
{
    FifoQueue<ProductionQueueEntry> queue;
    queue.Enqueue(ProductionQueueEntry(1, 100, 10, 20, 0.8, 1.0));
    queue.Enqueue(ProductionQueueEntry(2, 100, 15, 30, 0.8, 1.0));
    queue.Enqueue(ProductionQueueEntry(3, 100, 5, 10, 0.8, 1.0));

    const std::optional<ProductionQueueEntry> first = queue.PeekNext();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(first->GetId(), 1);
    ASSERT_TRUE(queue.DequeueCompleted());

    const std::optional<ProductionQueueEntry> second = queue.PeekNext();
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(second->GetId(), 2);
    ASSERT_TRUE(queue.DequeueCompleted());

    const std::optional<ProductionQueueEntry> third = queue.PeekNext();
    ASSERT_TRUE(third.has_value());
    EXPECT_EQ(third->GetId(), 3);
    ASSERT_TRUE(queue.DequeueCompleted());

    EXPECT_TRUE(queue.IsEmpty());
}
