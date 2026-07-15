// Phase 3 - MenuNode / ReadOnlyMenuNode / ActionMenuNode contract tests.
//
// docs/feature/controller.md section 2.1 (composite menu tree) and section 3
// (되돌아가기(N/0)는 항상 부작용 없이 취소되어야 한다).

#include <gtest/gtest.h>

#include "../Controller/MenuNode.h"
#include "Support/ConsoleRedirect.h"

namespace
{
    using ConsoleMVC::Controller::ActionMenuNode;
    using ConsoleMVC::Controller::ActionOutcome;
    using ConsoleMVC::Controller::IController;
    using ConsoleMVC::Controller::MenuNode;
    using ConsoleMVC::Controller::ReadOnlyMenuNode;
    using ConsoleMVC::Tests::ScopedStdinRedirect;
    using ConsoleMVC::Tests::ScopedStdoutCapture;
}

TEST(MenuNodeTest, AddChildSetsParentAndIsReflectedInGetChildren)
{
    auto parent = std::make_shared<MenuNode>("Parent");
    auto child = std::make_shared<MenuNode>("Child");

    parent->AddChild(child);

    ASSERT_EQ(parent->GetChildren().size(), 1u);
    EXPECT_EQ(parent->GetChildren()[0], child);
    EXPECT_EQ(child->GetParent(), parent.get());
}

TEST(MenuNodeTest, RootMenuHasNoParent)
{
    auto root = std::make_shared<MenuNode>("Root");

    EXPECT_EQ(root->GetParent(), nullptr);
}

TEST(MenuNodeTest, ContainerNodeReportsHasChildrenOnlyAfterAdding)
{
    auto container = std::make_shared<MenuNode>("Container");

    EXPECT_FALSE(container->HasChildren());

    container->AddChild(std::make_shared<MenuNode>("Child"));

    EXPECT_TRUE(container->HasChildren());
}

TEST(MenuNodeTest, ReadOnlyMenuNodeIsALeafWithNoChildren)
{
    ReadOnlyMenuNode view("List", [] {});

    EXPECT_FALSE(view.HasChildren());
    EXPECT_TRUE(view.GetChildren().empty());
}

TEST(MenuNodeTest, ReadOnlyMenuNodeExecuteRunsHandlerAndAlwaysSucceeds)
{
    bool handlerRan = false;
    ReadOnlyMenuNode view("List", [&handlerRan] { handlerRan = true; });

    const ActionOutcome outcome = view.Execute();

    EXPECT_TRUE(handlerRan);
    EXPECT_TRUE(outcome.succeeded);
}

TEST(MenuNodeTest, ActionMenuNodeWithoutConfirmationRunsHandlerImmediately)
{
    bool handlerRan = false;
    ActionMenuNode action("Do it", [&handlerRan]
    {
        handlerRan = true;
        return ActionOutcome{true, "done"};
    });

    const ActionOutcome outcome = action.Execute();

    EXPECT_TRUE(handlerRan);
    EXPECT_TRUE(outcome.succeeded);
    EXPECT_EQ(outcome.message, "done");
}

TEST(MenuNodeTest, ActionMenuNodeRunsHandlerWhenUserConfirmsWithY)
{
    ScopedStdinRedirect stdinRedirect("Y\n");
    ScopedStdoutCapture stdoutCapture;

    bool handlerRan = false;
    ActionMenuNode action(
        "Approve",
        [] { return std::vector<std::pair<std::string, std::string>>{{"Qty", "10"}}; },
        [&handlerRan]
        {
            handlerRan = true;
            return ActionOutcome{true, "approved"};
        });

    const ActionOutcome outcome = action.Execute();

    EXPECT_TRUE(handlerRan);
    EXPECT_TRUE(outcome.succeeded);
}

TEST(MenuNodeTest, ActionMenuNodeSkipsHandlerWhenUserCancelsWithN)
{
    ScopedStdinRedirect stdinRedirect("N\n");
    ScopedStdoutCapture stdoutCapture;

    bool handlerRan = false;
    ActionMenuNode action(
        "Approve",
        [] { return std::vector<std::pair<std::string, std::string>>{{"Qty", "10"}}; },
        [&handlerRan]
        {
            handlerRan = true;
            return ActionOutcome{true, "approved"};
        });

    const ActionOutcome outcome = action.Execute();

    EXPECT_FALSE(handlerRan) << "cancelling must never invoke the state-changing handler";
    EXPECT_TRUE(outcome.succeeded);
}

TEST(MenuNodeTest, ActionMenuNodeRePromptsUntilAValidYesNoAnswerIsGiven)
{
    // Invalid answer, then blank, then a valid "n".
    ScopedStdinRedirect stdinRedirect("maybe\n\nn\n");
    ScopedStdoutCapture stdoutCapture;

    bool handlerRan = false;
    ActionMenuNode action(
        "Approve",
        [] { return std::vector<std::pair<std::string, std::string>>{}; },
        [&handlerRan]
        {
            handlerRan = true;
            return ActionOutcome{true, ""};
        });

    const ActionOutcome outcome = action.Execute();

    EXPECT_FALSE(handlerRan);
    EXPECT_TRUE(outcome.succeeded);
}
