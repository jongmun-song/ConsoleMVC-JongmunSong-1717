// ExampleAppContext observer wiring integration test (PoC verification
// domain).
//
// docs/design/phase4.md item 2 ("모니터링 확장 지점 시연") / test point -
// "통합 시나리오 테스트(엔티티 추가 -> 상태 변경 -> Observer 알림 수신까지
// end-to-end)".
//
// ExampleAppContext.h documents that sampleModel/orderModel each get a
// ChangeLogObserver<T> subscribed at construction time purely through the
// core IModel<T>::Subscribe hook, without Controller/View being touched.
// This file is the formal gtest proof of that claim: it builds a real
// ExampleAppContext (not a hand-wired InMemoryModel + observer pair) and
// drives it through Add/Update on both Sample and Order, then asserts the
// context's own sampleChangeLog/orderChangeLog recorded the corresponding
// notifications - end to end, exactly as a future DataMonitor spike would
// observe it.

#include <gtest/gtest.h>

#include "../../Example/Controller/ExampleAppContext.h"
#include "../../Example/Model/Order.h"
#include "../../Example/Model/Sample.h"

namespace
{
    using ConsoleMVC::Example::Controller::ExampleAppContext;
    using ConsoleMVC::Example::Model::Order;
    using ConsoleMVC::Example::Model::OrderState;
    using ConsoleMVC::Example::Model::Sample;
}

TEST(ExampleAppContextObserverTest, SampleAddNotifiesTheContextsChangeLog)
{
    ExampleAppContext context;

    ASSERT_TRUE(context.sampleChangeLog.GetEntries().empty());

    context.sampleModel.Add(Sample(1, "Alpha", 1.0, 0.8, 5));

    const auto& entries = context.sampleChangeLog.GetEntries();
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_NE(entries.back().find("Sample"), std::string::npos);
    EXPECT_NE(entries.back().find("added"), std::string::npos);
    EXPECT_NE(entries.back().find("Alpha"), std::string::npos);
}

TEST(ExampleAppContextObserverTest, SampleUpdateAfterStockChangeNotifiesTheChangeLog)
{
    ExampleAppContext context;

    Sample sample(1, "Alpha", 1.0, 0.8, 5);
    context.sampleModel.Add(sample);
    ASSERT_EQ(context.sampleChangeLog.GetEntries().size(), 1u);

    // Entity state change (stock increase) followed by pushing the updated
    // value back through IModel<T>::Update, matching how OrderApprovalService
    // etc. propagate Model-side mutations - see docs/feature/model.md
    // section 6.4.
    ASSERT_TRUE(sample.TryIncreaseStock(3));
    ASSERT_TRUE(context.sampleModel.Update(sample));

    const auto& entries = context.sampleChangeLog.GetEntries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_NE(entries.back().find("updated"), std::string::npos);
    EXPECT_NE(entries.back().find("stock=8"), std::string::npos);
}

TEST(ExampleAppContextObserverTest, OrderAddAndStateTransitionNotifyTheChangeLog)
{
    ExampleAppContext context;

    ASSERT_TRUE(context.orderChangeLog.GetEntries().empty());

    Order order(1, /*sampleId=*/1, "Acme", 10);
    context.orderModel.Add(order);

    {
        const auto& entries = context.orderChangeLog.GetEntries();
        ASSERT_EQ(entries.size(), 1u);
        EXPECT_NE(entries.back().find("added"), std::string::npos);
        EXPECT_NE(entries.back().find("state=" + std::to_string(static_cast<int>(OrderState::RESERVED))),
            std::string::npos);
    }

    // End-to-end: entity add -> state transition -> Update -> Observer
    // notification, driven entirely through the core IModel<T>/StatefulModel
    // contracts the example domain builds on (docs/feature/model.md section
    // 6.2).
    ASSERT_TRUE(order.TryTransitionTo(OrderState::CONFIRMED));
    ASSERT_TRUE(context.orderModel.Update(order));

    const auto& entries = context.orderChangeLog.GetEntries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_NE(entries.back().find("updated"), std::string::npos);
    EXPECT_NE(entries.back().find("state=" + std::to_string(static_cast<int>(OrderState::CONFIRMED))),
        std::string::npos);
}

TEST(ExampleAppContextObserverTest, RemovingASampleNotifiesTheChangeLogWithLastKnownState)
{
    ExampleAppContext context;

    context.sampleModel.Add(Sample(7, "Gamma", 1.0, 0.8, 2));
    ASSERT_EQ(context.sampleChangeLog.GetEntries().size(), 1u);

    ASSERT_TRUE(context.sampleModel.Remove(7));

    const auto& entries = context.sampleChangeLog.GetEntries();
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_NE(entries.back().find("removed"), std::string::npos);
    EXPECT_NE(entries.back().find("Gamma"), std::string::npos);
}
