// Phase 1-B - FifoQueue<T> contract tests.
//
// docs/design/phase1.md "테스트 포인트 (tester)" section for the 1-B scope:
//   - FIFO ordering guarantee across Enqueue/PeekNext/DequeueCompleted
//   - PeekNext/DequeueCompleted behavior on an empty queue
//   - IsEmpty/Size bookkeeping as items are enqueued/dequeued
//
// The element type used here is plain int - this test does not encode any
// domain-specific queue semantics (e.g. a production/order queue).

#include <gtest/gtest.h>

#include "../Model/FifoQueue.h"

namespace
{
    using ConsoleMVC::Model::FifoQueue;
}

TEST(FifoQueueTest, NewQueueIsEmptyWithZeroSize)
{
    FifoQueue<int> queue;

    EXPECT_TRUE(queue.IsEmpty());
    EXPECT_EQ(queue.Size(), 0u);
}

TEST(FifoQueueTest, PeekNextOnEmptyQueueReturnsNullopt)
{
    FifoQueue<int> queue;

    EXPECT_EQ(queue.PeekNext(), std::nullopt);
}

TEST(FifoQueueTest, DequeueCompletedOnEmptyQueueReturnsFalse)
{
    FifoQueue<int> queue;

    EXPECT_FALSE(queue.DequeueCompleted());
}

TEST(FifoQueueTest, EnqueueThenPeekNextReturnsFrontItem)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);
    queue.Enqueue(2);

    const std::optional<int> next = queue.PeekNext();

    ASSERT_TRUE(next.has_value());
    EXPECT_EQ(next.value(), 1);
}

TEST(FifoQueueTest, PeekNextDoesNotRemoveItem)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);

    queue.PeekNext();

    EXPECT_EQ(queue.Size(), 1u);
    EXPECT_FALSE(queue.IsEmpty());
}

TEST(FifoQueueTest, ItemsAreDequeuedInFifoOrder)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);
    queue.Enqueue(2);
    queue.Enqueue(3);

    ASSERT_EQ(queue.PeekNext(), 1);
    ASSERT_TRUE(queue.DequeueCompleted());

    ASSERT_EQ(queue.PeekNext(), 2);
    ASSERT_TRUE(queue.DequeueCompleted());

    ASSERT_EQ(queue.PeekNext(), 3);
    ASSERT_TRUE(queue.DequeueCompleted());

    EXPECT_TRUE(queue.IsEmpty());
}

TEST(FifoQueueTest, SizeReflectsEnqueueAndDequeueOperations)
{
    FifoQueue<int> queue;
    EXPECT_EQ(queue.Size(), 0u);

    queue.Enqueue(10);
    EXPECT_EQ(queue.Size(), 1u);

    queue.Enqueue(20);
    EXPECT_EQ(queue.Size(), 2u);

    ASSERT_TRUE(queue.DequeueCompleted());
    EXPECT_EQ(queue.Size(), 1u);

    ASSERT_TRUE(queue.DequeueCompleted());
    EXPECT_EQ(queue.Size(), 0u);
    EXPECT_TRUE(queue.IsEmpty());
}

TEST(FifoQueueTest, DequeueCompletedAfterQueueDrainedReturnsFalse)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);
    ASSERT_TRUE(queue.DequeueCompleted());

    EXPECT_FALSE(queue.DequeueCompleted());
    EXPECT_TRUE(queue.IsEmpty());
}
