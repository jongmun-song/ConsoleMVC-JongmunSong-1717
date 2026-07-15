// Example/Model/Order.h contract tests (PoC verification domain).
//
// docs/feature/model.md section 6.2 - Order is IEntity<int> + core
// StatefulModel<OrderState>, and must integrate with the core
// ConsoleMVC::Model::InMemoryModel<T> unchanged.
//
// Transition table under test (see Order.h IsTransitionAllowed):
//   RESERVED  -> CONFIRMED | PRODUCING | REJECTED   (valid)
//   PRODUCING -> CONFIRMED                          (valid)
//   CONFIRMED -> RELEASE                            (valid)
//   RELEASE, REJECTED are terminal - everything else is rejected.
//
// The invalid-transition cases below are a regression guard for this
// review cycle's state-graph fix: PRODUCING->REJECTED and
// CONFIRMED->REJECTED must NOT be allowed (rejection is only ever
// performed directly from RESERVED), and the terminal states RELEASE /
// REJECTED must reject every further transition.

#include <gtest/gtest.h>

#include "../../Example/Model/Order.h"
#include "../../Model/InMemoryModel.h"

namespace
{
    using ConsoleMVC::Example::Model::Order;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Model::InMemoryModel;
}

TEST(OrderTest, NewOrderStartsInReservedState)
{
    Order order(1, 100, "Customer A", 5);

    EXPECT_EQ(order.GetState(), OrderState::RESERVED);
}

TEST(OrderTest, ConstructorFieldsAreReturnedAsGiven)
{
    Order order(1, 100, "Customer A", 5);

    EXPECT_EQ(order.GetId(), 1);
    EXPECT_EQ(order.GetSampleId(), 100);
    EXPECT_EQ(order.GetCustomerName(), "Customer A");
    EXPECT_EQ(order.GetOrderedQuantity(), 5);
}

TEST(OrderTest, ReservedToConfirmedTransitionSucceeds)
{
    Order order(1, 100, "Customer A", 5);

    const bool result = order.TryTransitionTo(OrderState::CONFIRMED);

    EXPECT_TRUE(result);
    EXPECT_EQ(order.GetState(), OrderState::CONFIRMED);
}

TEST(OrderTest, ReservedToProducingTransitionSucceeds)
{
    Order order(1, 100, "Customer A", 5);

    const bool result = order.TryTransitionTo(OrderState::PRODUCING);

    EXPECT_TRUE(result);
    EXPECT_EQ(order.GetState(), OrderState::PRODUCING);
}

TEST(OrderTest, ReservedToRejectedTransitionSucceeds)
{
    Order order(1, 100, "Customer A", 5);

    const bool result = order.TryTransitionTo(OrderState::REJECTED);

    EXPECT_TRUE(result);
    EXPECT_EQ(order.GetState(), OrderState::REJECTED);
}

TEST(OrderTest, ProducingToConfirmedTransitionSucceeds)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::PRODUCING));

    const bool result = order.TryTransitionTo(OrderState::CONFIRMED);

    EXPECT_TRUE(result);
    EXPECT_EQ(order.GetState(), OrderState::CONFIRMED);
}

TEST(OrderTest, ConfirmedToReleaseTransitionSucceeds)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::CONFIRMED));

    const bool result = order.TryTransitionTo(OrderState::RELEASE);

    EXPECT_TRUE(result);
    EXPECT_EQ(order.GetState(), OrderState::RELEASE);
}

// --- Regression: invalid transitions must be rejected and leave state
// unchanged. ---

TEST(OrderTest, ProducingToRejectedTransitionFailsAndLeavesStateUnchanged)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::PRODUCING));

    const bool result = order.TryTransitionTo(OrderState::REJECTED);

    EXPECT_FALSE(result);
    EXPECT_EQ(order.GetState(), OrderState::PRODUCING);
}

TEST(OrderTest, ConfirmedToRejectedTransitionFailsAndLeavesStateUnchanged)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::CONFIRMED));

    const bool result = order.TryTransitionTo(OrderState::REJECTED);

    EXPECT_FALSE(result);
    EXPECT_EQ(order.GetState(), OrderState::CONFIRMED);
}

TEST(OrderTest, ReleaseIsTerminalAndRejectsAnyFurtherTransition)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::CONFIRMED));
    ASSERT_TRUE(order.TryTransitionTo(OrderState::RELEASE));

    EXPECT_FALSE(order.TryTransitionTo(OrderState::RESERVED));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::CONFIRMED));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::PRODUCING));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::REJECTED));
    EXPECT_EQ(order.GetState(), OrderState::RELEASE);
}

TEST(OrderTest, RejectedIsTerminalAndRejectsAnyFurtherTransition)
{
    Order order(1, 100, "Customer A", 5);
    ASSERT_TRUE(order.TryTransitionTo(OrderState::REJECTED));

    EXPECT_FALSE(order.TryTransitionTo(OrderState::RESERVED));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::CONFIRMED));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::PRODUCING));
    EXPECT_FALSE(order.TryTransitionTo(OrderState::RELEASE));
    EXPECT_EQ(order.GetState(), OrderState::REJECTED);
}

TEST(OrderTest, InMemoryModelAddGetByIdIntegration)
{
    InMemoryModel<Order> model;
    model.Add(Order(1, 100, "Customer A", 5));
    model.Add(Order(2, 200, "Customer B", 3));

    const std::optional<Order> found = model.GetById(2);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetCustomerName(), "Customer B");
    EXPECT_EQ(found->GetState(), OrderState::RESERVED);
}
