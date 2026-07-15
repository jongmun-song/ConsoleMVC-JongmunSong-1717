// Example/Model/Sample.h contract tests (PoC verification domain).
//
// docs/feature/model.md section 6.1 - Sample is IEntity<int> + core
// QuantityGuard<int>, and must integrate with the core
// ConsoleMVC::Model::InMemoryModel<T> unchanged.
//
// docs/design/phase1.md item 7 test points:
//   - constructor fields (id/name/avg production time/yield/initial stock)
//     are returned as given
//   - TryIncreaseStock increases stock
//   - TryDecreaseStock decreases stock; decreasing by more than the current
//     stock fails (false) and leaves stock unchanged (QuantityGuard
//     invariant, never negative)
//   - InMemoryModel<Sample> Add/GetById/GetAll integration

#include <gtest/gtest.h>

#include "../../Example/Model/Sample.h"
#include "../../Model/InMemoryModel.h"

namespace
{
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Model::InMemoryModel;
}

TEST(SampleTest, ConstructorFieldsAreReturnedAsGiven)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    EXPECT_EQ(sample.GetId(), 1);
    EXPECT_EQ(sample.GetName(), "Test Sample");
    EXPECT_DOUBLE_EQ(sample.GetAverageProductionTimePerUnit(), 2.5);
    EXPECT_DOUBLE_EQ(sample.GetYieldRatio(), 0.9);
    EXPECT_EQ(sample.GetStockQuantity(), 10);
}

TEST(SampleTest, DefaultInitialStockQuantityIsZero)
{
    Sample sample(1, "Test Sample", 2.5, 0.9);

    EXPECT_EQ(sample.GetStockQuantity(), 0);
}

TEST(SampleTest, TryIncreaseStockIncreasesStock)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    const bool result = sample.TryIncreaseStock(5);

    EXPECT_TRUE(result);
    EXPECT_EQ(sample.GetStockQuantity(), 15);
}

TEST(SampleTest, TryDecreaseStockWithinBoundsDecreasesStock)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    const bool result = sample.TryDecreaseStock(4);

    EXPECT_TRUE(result);
    EXPECT_EQ(sample.GetStockQuantity(), 6);
}

TEST(SampleTest, TryDecreaseStockBeyondCurrentStockFailsAndLeavesStockUnchanged)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    const bool result = sample.TryDecreaseStock(11);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetStockQuantity(), 10);
}

// --- Reservation contract (docs/feature/model.md section 6.4, rule 4) ---
// Available quantity = stock - reserved; TryReserve/TryReleaseReservation/
// TryFulfillReservation are the only ways to move that reserved amount.

TEST(SampleTest, GetAvailableQuantityIsStockMinusReserved)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    EXPECT_EQ(sample.GetAvailableQuantity(), 10);

    ASSERT_TRUE(sample.TryReserve(4));
    EXPECT_EQ(sample.GetReservedQuantity(), 4);
    EXPECT_EQ(sample.GetAvailableQuantity(), 6);
}

TEST(SampleTest, GetAvailableQuantityIsZeroWhenReservedEqualsStock)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    ASSERT_TRUE(sample.TryReserve(10));

    EXPECT_EQ(sample.GetAvailableQuantity(), 0);
}

TEST(SampleTest, TryReserveWithinAvailableQuantitySucceeds)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    const bool result = sample.TryReserve(10);

    EXPECT_TRUE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 10);
    EXPECT_EQ(sample.GetStockQuantity(), 10);
}

TEST(SampleTest, TryReserveBeyondAvailableQuantityFailsAndLeavesReservedUnchanged)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(3));

    const bool result = sample.TryReserve(8); // available is only 7

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 3);
    EXPECT_EQ(sample.GetAvailableQuantity(), 7);
}

TEST(SampleTest, TryReserveNegativeAmountFails)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);

    const bool result = sample.TryReserve(-1);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 0);
}

TEST(SampleTest, TryReleaseReservationDecreasesReserved)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(6));

    const bool result = sample.TryReleaseReservation(4);

    EXPECT_TRUE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 2);
    EXPECT_EQ(sample.GetStockQuantity(), 10); // actual stock is untouched
    EXPECT_EQ(sample.GetAvailableQuantity(), 8);
}

TEST(SampleTest, TryReleaseReservationBeyondReservedFailsAndLeavesReservedUnchanged)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(5));

    const bool result = sample.TryReleaseReservation(6);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 5);
}

TEST(SampleTest, TryFulfillReservationDecreasesBothStockAndReserved)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(7));

    const bool result = sample.TryFulfillReservation(7);

    EXPECT_TRUE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 0);
    EXPECT_EQ(sample.GetStockQuantity(), 3);
    EXPECT_EQ(sample.GetAvailableQuantity(), 3);
}

TEST(SampleTest, TryFulfillReservationBeyondReservedFailsAndLeavesBothUnchanged)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(4));

    const bool result = sample.TryFulfillReservation(5);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 4);
    EXPECT_EQ(sample.GetStockQuantity(), 10);
}

TEST(SampleTest, TryFulfillReservationBeyondActualStockFailsAndRollsBackReservedDecrease)
{
    // Deliberately inconsistent state: reserved (8) ends up greater than
    // actual stock (5) after a direct TryDecreaseStock call that does not
    // know about reservations. TryFulfillReservation's defensive stock
    // check must reject this and leave both fields exactly as they were.
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(8));
    ASSERT_TRUE(sample.TryDecreaseStock(5)); // stock: 10 -> 5, reserved stays 8

    const bool result = sample.TryFulfillReservation(8);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 8);
    EXPECT_EQ(sample.GetStockQuantity(), 5);
}

TEST(SampleTest, TryFulfillReservationNegativeAmountFails)
{
    Sample sample(1, "Test Sample", 2.5, 0.9, 10);
    ASSERT_TRUE(sample.TryReserve(5));

    const bool result = sample.TryFulfillReservation(-1);

    EXPECT_FALSE(result);
    EXPECT_EQ(sample.GetReservedQuantity(), 5);
    EXPECT_EQ(sample.GetStockQuantity(), 10);
}

TEST(SampleTest, InMemoryModelAddGetByIdGetAllIntegration)
{
    InMemoryModel<Sample> model;
    model.Add(Sample(1, "Alpha", 1.0, 0.8, 5));
    model.Add(Sample(2, "Beta", 2.0, 0.7, 3));

    const std::optional<Sample> found = model.GetById(1);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetName(), "Alpha");
    EXPECT_EQ(found->GetStockQuantity(), 5);

    const std::vector<Sample> all = model.GetAll();
    ASSERT_EQ(all.size(), 2u);
}
