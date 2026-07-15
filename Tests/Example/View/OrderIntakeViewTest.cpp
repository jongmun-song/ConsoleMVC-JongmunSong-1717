// Example/View/OrderIntakeView.h contract tests (PoC verification domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// OrderIntakeView renders sample id / customer name / ordered quantity via
// ConfirmView, without requiring a constructed Order.

#include <gtest/gtest.h>

#include "../../../Example/View/OrderIntakeView.h"

namespace
{
    using ConsoleMVC::Example::View::OrderIntakeView;
}

TEST(OrderIntakeViewTest, RenderConfirmationContainsSampleIdCustomerAndQuantity)
{
    const std::string result = OrderIntakeView::RenderConfirmation(42, "Acme Corp", 15);

    EXPECT_NE(result.find("42"), std::string::npos);
    EXPECT_NE(result.find("Acme Corp"), std::string::npos);
    EXPECT_NE(result.find("15"), std::string::npos);
}

TEST(OrderIntakeViewTest, RenderConfirmationUsesDefaultPromptWhenNotSpecified)
{
    const std::string result = OrderIntakeView::RenderConfirmation(1, "Customer A", 1);

    EXPECT_NE(result.find("Y/N"), std::string::npos);
}

TEST(OrderIntakeViewTest, RenderConfirmationReflectsCustomPrompt)
{
    const std::string result = OrderIntakeView::RenderConfirmation(1, "Customer A", 1, "Confirm order? ");

    EXPECT_NE(result.find("Confirm order? "), std::string::npos);
}
