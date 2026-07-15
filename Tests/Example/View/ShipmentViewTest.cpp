// Example/View/ShipmentView.h contract tests (PoC verification domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// ShipmentView renders a ConfirmView release prompt and a MessageView
// outcome (Success/Failure prefixes verified against
// Tests/MessageViewTest.cpp).

#include <gtest/gtest.h>

#include "../../../Example/View/ShipmentView.h"

namespace
{
    using ConsoleMVC::Example::View::ShipmentView;
}

TEST(ShipmentViewTest, RenderConfirmationContainsOrderIdSampleNameAndQuantity)
{
    const std::string result = ShipmentView::RenderConfirmation(11, "Alpha", 25);

    EXPECT_NE(result.find("11"), std::string::npos);
    EXPECT_NE(result.find("Alpha"), std::string::npos);
    EXPECT_NE(result.find("25"), std::string::npos);
}

TEST(ShipmentViewTest, RenderConfirmationUsesDefaultPromptWhenNotSpecified)
{
    const std::string result = ShipmentView::RenderConfirmation(1, "Alpha", 1);

    EXPECT_NE(result.find("Y/N"), std::string::npos);
}

TEST(ShipmentViewTest, RenderConfirmationReflectsCustomPrompt)
{
    const std::string result = ShipmentView::RenderConfirmation(1, "Alpha", 1, "Ship now? ");

    EXPECT_NE(result.find("Ship now? "), std::string::npos);
}

TEST(ShipmentViewTest, RenderResultSucceededUsesSuccessMessageStyle)
{
    const std::string result = ShipmentView::RenderResult(true, "Shipment released");

    EXPECT_EQ(result, "[OK] Shipment released");
}

TEST(ShipmentViewTest, RenderResultFailedUsesFailureMessageStyle)
{
    const std::string result = ShipmentView::RenderResult(false, "Shipment failed");

    EXPECT_EQ(result, "[FAIL] Shipment failed");
}
