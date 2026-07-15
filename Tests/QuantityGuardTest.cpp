// Phase 1-B - QuantityGuard<TQuantity> contract tests.
//
// docs/design/phase1.md "테스트 포인트 (tester)" section for the 1-B scope:
//   - Boundary values for QuantityGuard: reaching exactly zero, attempting a
//     negative amount
//
// docs/feature/model.md section 3.4 - the guarded quantity must never drop
// below zero, and a rejected TryIncrease/TryDecrease must leave the quantity
// unchanged (ordinary failure, not an exception).

#include <gtest/gtest.h>

#include "../Model/QuantityGuard.h"

namespace
{
    using ConsoleMVC::Model::QuantityGuard;
}

TEST(QuantityGuardTest, InitialQuantityDefaultsToZero)
{
    QuantityGuard<int> guard;

    EXPECT_EQ(guard.GetQuantity(), 0);
}

TEST(QuantityGuardTest, InitialQuantityMatchesConstructorArgument)
{
    QuantityGuard<int> guard(10);

    EXPECT_EQ(guard.GetQuantity(), 10);
}

TEST(QuantityGuardTest, TryIncreaseWithPositiveAmountSucceeds)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryIncrease(3);

    EXPECT_TRUE(result);
    EXPECT_EQ(guard.GetQuantity(), 8);
}

TEST(QuantityGuardTest, TryIncreaseWithNegativeAmountFailsAndLeavesQuantityUnchanged)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryIncrease(-1);

    EXPECT_FALSE(result);
    EXPECT_EQ(guard.GetQuantity(), 5);
}

TEST(QuantityGuardTest, TryDecreaseWithinBoundsSucceeds)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryDecrease(2);

    EXPECT_TRUE(result);
    EXPECT_EQ(guard.GetQuantity(), 3);
}

TEST(QuantityGuardTest, TryDecreaseExactlyToZeroSucceeds)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryDecrease(5);

    EXPECT_TRUE(result);
    EXPECT_EQ(guard.GetQuantity(), 0);
}

TEST(QuantityGuardTest, TryDecreaseBelowZeroFailsAndLeavesQuantityUnchanged)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryDecrease(6);

    EXPECT_FALSE(result);
    EXPECT_EQ(guard.GetQuantity(), 5);
}

TEST(QuantityGuardTest, TryDecreaseWithNegativeAmountFailsAndLeavesQuantityUnchanged)
{
    QuantityGuard<int> guard(5);

    const bool result = guard.TryDecrease(-1);

    EXPECT_FALSE(result);
    EXPECT_EQ(guard.GetQuantity(), 5);
}

TEST(QuantityGuardTest, TryDecreaseOnZeroQuantityWithPositiveAmountFails)
{
    QuantityGuard<int> guard(0);

    const bool result = guard.TryDecrease(1);

    EXPECT_FALSE(result);
    EXPECT_EQ(guard.GetQuantity(), 0);
}

TEST(QuantityGuardTest, TryDecreaseZeroAmountOnZeroQuantitySucceeds)
{
    QuantityGuard<int> guard(0);

    const bool result = guard.TryDecrease(0);

    EXPECT_TRUE(result);
    EXPECT_EQ(guard.GetQuantity(), 0);
}
