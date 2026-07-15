// Phase 2-A - MessageView contract tests.
//
// docs/design/phase2.md "테스트 포인트 (tester)" section for this scope:
//   - Success/Failure/Info styles render distinct prefixes
//
// Uses generic message strings only - MessageStyle is a generic
// classification, not a domain-specific result code.

#include <gtest/gtest.h>

#include "../View/MessageView.h"

namespace
{
    using ConsoleMVC::View::MessageStyle;
    using ConsoleMVC::View::MessageView;
}

TEST(MessageViewTest, SuccessStyleAddsOkPrefix)
{
    const std::string result = MessageView::Render("Operation done", MessageStyle::Success);

    EXPECT_EQ(result, "[OK] Operation done");
}

TEST(MessageViewTest, FailureStyleAddsFailPrefix)
{
    const std::string result = MessageView::Render("Operation failed", MessageStyle::Failure);

    EXPECT_EQ(result, "[FAIL] Operation failed");
}

TEST(MessageViewTest, InfoStyleAddsInfoPrefix)
{
    const std::string result = MessageView::Render("Just a note", MessageStyle::Info);

    EXPECT_EQ(result, "[INFO] Just a note");
}

TEST(MessageViewTest, DefaultStyleIsInfo)
{
    const std::string result = MessageView::Render("Just a note");

    EXPECT_EQ(result, MessageView::Render("Just a note", MessageStyle::Info));
}

TEST(MessageViewTest, EachStyleProducesADistinctPrefix)
{
    const std::string success = MessageView::Render("Same message", MessageStyle::Success);
    const std::string failure = MessageView::Render("Same message", MessageStyle::Failure);
    const std::string info = MessageView::Render("Same message", MessageStyle::Info);

    EXPECT_NE(success, failure);
    EXPECT_NE(success, info);
    EXPECT_NE(failure, info);
}

TEST(MessageViewTest, MessageTextIsPreservedVerbatim)
{
    const std::string result = MessageView::Render("Arbitrary message 456", MessageStyle::Failure);

    EXPECT_NE(result.find("Arbitrary message 456"), std::string::npos);
}
