// Example/View/SampleRegistrationView.h contract tests (PoC verification
// domain).
//
// docs/design/phase2.md item 8 test point: the example screens render the
// expected output for Example/Model data.
//
// SampleRegistrationView adapts a not-yet-persisted Sample into a
// ConfirmView key/value summary.

#include <gtest/gtest.h>

#include "../../../Example/View/SampleRegistrationView.h"

namespace
{
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Example::View::SampleRegistrationView;
}

TEST(SampleRegistrationViewTest, RenderConfirmationContainsSampleFields)
{
    const Sample sample(7, "Gamma", 3.25, 0.75, 20);

    const std::string result = SampleRegistrationView::RenderConfirmation(sample);

    EXPECT_NE(result.find("7"), std::string::npos);
    EXPECT_NE(result.find("Gamma"), std::string::npos);
    EXPECT_NE(result.find("3.25"), std::string::npos);
    EXPECT_NE(result.find("75%"), std::string::npos);
    EXPECT_NE(result.find("20"), std::string::npos);
}

TEST(SampleRegistrationViewTest, RenderConfirmationUsesDefaultPromptWhenNotSpecified)
{
    const Sample sample(1, "Alpha", 1.0, 0.9, 1);

    const std::string result = SampleRegistrationView::RenderConfirmation(sample);

    EXPECT_NE(result.find("Y/N"), std::string::npos);
}

TEST(SampleRegistrationViewTest, RenderConfirmationReflectsCustomPrompt)
{
    const Sample sample(1, "Alpha", 1.0, 0.9, 1);

    const std::string result = SampleRegistrationView::RenderConfirmation(sample, "Save it? ");

    EXPECT_NE(result.find("Save it? "), std::string::npos);
}
