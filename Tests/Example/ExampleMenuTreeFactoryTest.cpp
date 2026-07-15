// End-to-end tests for the example menu tree (Example/Controller/*), Phase
// 3-B. Fills the coverage gap the code review flagged: the menu tree
// (ExampleMenuTreeFactory + Sample/Order/Monitoring/Production/Shipment
// factories) is not yet wired into main.cpp, so nothing had exercised it
// through an actual Controller::NavigationLoop with scripted console input.
//
// Each test builds ExampleMenuTreeFactory::BuildMainMenu(context), drives it
// with ScopedStdinRedirect/ScopedStdoutCapture (Tests/Support/
// ConsoleRedirect.h, the same helper Tests/NavigationLoopTest.cpp uses), and
// asserts on the resulting Model state in `context` - not on the exact
// rendered text, matching this repo's existing NavigationLoopTest style.
//
// Main Menu item numbers (see ExampleMenuTreeFactory::BuildMainMenu):
//   1 = Sample Management      (submenu: 1 = List, 2 = Register)
//   2 = Place a new order      (leaf)
//   3 = Order Approval / Rejection (submenu: 1 = Approve, 2 = Reject)
//   4 = Monitoring             (leaf)
//   5 = Production Line        (submenu: 1 = Show queue, 2 = Advance)
//   6 = Release a shipment     (leaf)
//   0 = Back / Exit

#include <gtest/gtest.h>

#include "../../Controller/NavigationLoop.h"
#include "../../Example/Controller/ExampleAppContext.h"
#include "../../Example/Controller/ExampleMenuTreeFactory.h"
#include "../../Example/Model/Order.h"
#include "../../Example/Model/Sample.h"
#include "../Support/ConsoleRedirect.h"

namespace
{
    using ConsoleMVC::Controller::NavigationLoop;
    using ConsoleMVC::Example::Controller::ExampleAppContext;
    using ConsoleMVC::Example::Controller::ExampleMenuTreeFactory;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Tests::ScopedStdinRedirect;
    using ConsoleMVC::Tests::ScopedStdoutCapture;

    void RunScript(ExampleAppContext& context, const std::string& scriptedInput)
    {
        ScopedStdinRedirect stdinRedirect(scriptedInput);
        ScopedStdoutCapture stdoutCapture;

        NavigationLoop loop(ExampleMenuTreeFactory::BuildMainMenu(context));
        loop.Run();
    }
}

// Full happy-path scenario: register a sample with enough stock to cover an
// order outright, place the order, approve it (straight to CONFIRMED, no
// production needed), then release it for shipment.
TEST(ExampleMenuTreeFactoryTest, RegisterOrderApproveAndReleaseFlowsThroughWithoutProduction)
{
    ExampleAppContext context;

    RunScript(
        context,
        "1\n"          // Main Menu -> Sample Management
        "2\n"          // Register a new sample
        "Widget\n"     // name
        "1.0\n"        // average production time per unit
        "1.0\n"        // yield ratio
        "100\n"        // initial stock
        "Y\n"          // confirm registration
        "0\n"          // back to Main Menu
        "2\n"          // Place a new order
        "1\n"          // sample id = 1
        "Customer A\n" // customer name
        "50\n"         // ordered quantity (<= available stock 100)
        "Y\n"          // confirm placement
        "3\n"          // Main Menu -> Order Approval / Rejection
        "1\n"          // Approve a reserved order
        "1\n"          // order id = 1
        "Y\n"          // confirm approval
        "0\n"          // back to Main Menu
        "6\n"          // Release a shipment
        "1\n"          // order id = 1
        "Y\n"          // confirm release
        "0\n");        // exit

    const auto sample = context.sampleModel.GetById(1);
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->GetStockQuantity(), 50); // 100 - 50 released
    EXPECT_EQ(sample->GetReservedQuantity(), 0);

    const auto order = context.orderModel.GetById(1);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->GetState(), OrderState::RELEASE);
}

// Full happy-path scenario with insufficient stock: approval routes the
// order through PRODUCING, the production line must be advanced twice
// (start, then complete) before the order becomes CONFIRMED and can be
// released.
TEST(ExampleMenuTreeFactoryTest, RegisterOrderRequiringProductionFlowsThroughToRelease)
{
    ExampleAppContext context;

    RunScript(
        context,
        "1\n"          // Sample Management
        "2\n"          // Register a new sample
        "Gadget\n"     // name
        "1.0\n"        // average production time per unit
        "0.5\n"        // yield ratio
        "0\n"          // initial stock (none available)
        "Y\n"          // confirm registration
        "0\n"          // back to Main Menu
        "2\n"          // Place a new order
        "1\n"          // sample id = 1
        "Customer B\n" // customer name
        "10\n"         // ordered quantity (> available stock 0)
        "Y\n"          // confirm placement
        "3\n"          // Order Approval / Rejection
        "1\n"          // Approve a reserved order
        "1\n"          // order id = 1
        "Y\n"          // confirm approval -> PRODUCING, shortage queued
        "0\n"          // back to Main Menu
        "5\n"          // Production Line
        "2\n"          // Advance (WAITING -> PRODUCING)
        "2\n"          // Advance again (PRODUCING -> CONFIRMED; stock/order updated)
        "0\n"          // back to Main Menu
        "6\n"          // Release a shipment
        "1\n"          // order id = 1
        "Y\n"          // confirm release
        "0\n");        // exit

    const auto sample = context.sampleModel.GetById(1);
    ASSERT_TRUE(sample.has_value());
    // Production added ceil(10 / 0.5) = 20 units; release then fulfils the
    // full ordered quantity (10), leaving 10 in stock.
    EXPECT_EQ(sample->GetStockQuantity(), 10);
    EXPECT_EQ(sample->GetReservedQuantity(), 0);

    const auto order = context.orderModel.GetById(1);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->GetState(), OrderState::RELEASE);
}

// docs/feature/controller.md "되돌아가기(N/0)는 항상 부작용 없이 취소되어야
// 한다", exercised at the actual menu-tree level (not just the generic
// NavigationLoopTest demo tree): answering N to the registration
// confirmation must leave ExampleAppContext::sampleModel completely empty.
TEST(ExampleMenuTreeFactoryTest, CancellingSampleRegistrationWithNLeavesSampleModelUnchanged)
{
    ExampleAppContext context;

    RunScript(
        context,
        "1\n"      // Sample Management
        "2\n"      // Register a new sample
        "Widget\n" // name
        "1.0\n"    // average production time per unit
        "1.0\n"    // yield ratio
        "100\n"    // initial stock
        "N\n"      // cancel registration
        "0\n"      // back to Main Menu
        "0\n");    // exit

    EXPECT_TRUE(context.sampleModel.GetAll().empty());
    EXPECT_EQ(context.nextSampleId, 1); // id counter must not have advanced either
}

// Same cancellation contract, but for order approval: answering N to the
// approval confirmation must leave the order RESERVED and the sample's
// reservation untouched.
TEST(ExampleMenuTreeFactoryTest, CancellingOrderApprovalWithNLeavesOrderAndSampleUnchanged)
{
    ExampleAppContext context;

    RunScript(
        context,
        "1\n"          // Sample Management
        "2\n"          // Register a new sample
        "Widget\n"
        "1.0\n"
        "1.0\n"
        "100\n"
        "Y\n"
        "0\n"
        "2\n"          // Place a new order
        "1\n"
        "Customer A\n"
        "50\n"
        "Y\n"
        "3\n"          // Order Approval / Rejection
        "1\n"          // Approve a reserved order
        "1\n"          // order id = 1
        "N\n"          // cancel approval
        "0\n"
        "0\n");

    const auto order = context.orderModel.GetById(1);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->GetState(), OrderState::RESERVED);

    const auto sample = context.sampleModel.GetById(1);
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->GetReservedQuantity(), 0);
    EXPECT_EQ(sample->GetStockQuantity(), 100);
}
