// Example/View/OrderApprovalView.h contract tests (PoC verification
// domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// Status -> BadgeStyle mapping under test (see OrderApprovalView::StatusBadge
// and Tests/BadgeViewTest.cpp for the marker strings each style produces):
//   RESERVED/CONFIRMED/RELEASE -> Normal  ("[LABEL]", no marker)
//   PRODUCING                  -> Warning ("[! LABEL]")
//   REJECTED                   -> Error   ("[!! LABEL]")

#include <gtest/gtest.h>

#include "../../../Example/View/OrderApprovalView.h"

namespace
{
    using ConsoleMVC::Example::Model::Order;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Example::View::OrderApprovalView;

    Order MakeOrderInState(int id, int sampleId, const std::string& customer, int quantity, OrderState state)
    {
        Order order(id, sampleId, customer, quantity);
        if (state == OrderState::RESERVED)
        {
            return order;
        }
        if (state == OrderState::PRODUCING)
        {
            order.TryTransitionTo(OrderState::PRODUCING);
            return order;
        }
        if (state == OrderState::CONFIRMED)
        {
            order.TryTransitionTo(OrderState::CONFIRMED);
            return order;
        }
        if (state == OrderState::RELEASE)
        {
            order.TryTransitionTo(OrderState::CONFIRMED);
            order.TryTransitionTo(OrderState::RELEASE);
            return order;
        }
        // REJECTED
        order.TryTransitionTo(OrderState::REJECTED);
        return order;
    }
}

TEST(OrderApprovalViewTest, RenderPendingOrdersContainsEachOrderFieldInOrder)
{
    const std::vector<Order> orders = {
        MakeOrderInState(1, 100, "Customer A", 5, OrderState::RESERVED),
        MakeOrderInState(2, 200, "Customer B", 8, OrderState::RESERVED),
    };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("Customer A"), std::string::npos);
    EXPECT_NE(result.find("Customer B"), std::string::npos);
    EXPECT_LT(result.find("Customer A"), result.find("Customer B"));
}

TEST(OrderApprovalViewTest, RenderPendingOrdersBadgesReservedAsNormal)
{
    const std::vector<Order> orders = { MakeOrderInState(1, 100, "Customer A", 5, OrderState::RESERVED) };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("[RESERVED]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderPendingOrdersBadgesConfirmedAsNormal)
{
    const std::vector<Order> orders = { MakeOrderInState(1, 100, "Customer A", 5, OrderState::CONFIRMED) };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("[CONFIRMED]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderPendingOrdersBadgesReleaseAsNormal)
{
    const std::vector<Order> orders = { MakeOrderInState(1, 100, "Customer A", 5, OrderState::RELEASE) };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("[RELEASE]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderPendingOrdersBadgesProducingAsWarning)
{
    const std::vector<Order> orders = { MakeOrderInState(1, 100, "Customer A", 5, OrderState::PRODUCING) };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("[! PRODUCING]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderPendingOrdersBadgesRejectedAsError)
{
    const std::vector<Order> orders = { MakeOrderInState(1, 100, "Customer A", 5, OrderState::REJECTED) };

    const std::string result = OrderApprovalView::RenderPendingOrders(orders);

    EXPECT_NE(result.find("[!! REJECTED]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderDecisionConfirmationContainsOrderFieldsAndStatusBadge)
{
    const Order order = MakeOrderInState(3, 100, "Customer C", 9, OrderState::PRODUCING);

    const std::string result = OrderApprovalView::RenderDecisionConfirmation(order);

    EXPECT_NE(result.find("3"), std::string::npos);
    EXPECT_NE(result.find("100"), std::string::npos);
    EXPECT_NE(result.find("Customer C"), std::string::npos);
    EXPECT_NE(result.find("9"), std::string::npos);
    EXPECT_NE(result.find("[! PRODUCING]"), std::string::npos);
}

TEST(OrderApprovalViewTest, RenderDecisionConfirmationReflectsCustomPrompt)
{
    const Order order = MakeOrderInState(1, 100, "Customer A", 5, OrderState::RESERVED);

    const std::string result = OrderApprovalView::RenderDecisionConfirmation(order, "Reject this order? (Y/N) > ");

    EXPECT_NE(result.find("Reject this order? (Y/N) > "), std::string::npos);
}
