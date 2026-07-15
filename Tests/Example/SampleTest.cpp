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
