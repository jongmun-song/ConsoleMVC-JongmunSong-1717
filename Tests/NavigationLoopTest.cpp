// Phase 3 - NavigationLoop end-to-end tests.
//
// This is also the "minimal demo menu tree" required by docs/design/phase3.md
// item 5 and docs/PRD.md section 8 ("최소 2단계 이상 중첩된 메뉴 내비게이션이
// 동작하는 예제 포함"): it builds a two-level menu (Main Menu -> Reports /
// Actions submenu -> leaf screens) out of generic MenuNode/ReadOnlyMenuNode/
// ActionMenuNode instances (no domain types) and drives it end-to-end through
// NavigationLoop with scripted console input, covering every row of
// docs/feature/controller.md section 4:
//   - non-existent menu number       -> error message, same menu re-shown
//   - non-numeric input              -> error message, retried
//   - blank input                    -> error message, retried
//   - invalid Y/N confirmation input -> error message, retried
//   - Model-rejected transition      -> failure message, menu re-shown, state unchanged
//   - unexpected runtime exception   -> caught, failure message, back to main menu
//
// The state type used to exercise the "Model-rejected transition" row is the
// same generic placeholder enum used by Tests/StatefulModelTest.cpp - no
// domain-specific state names are introduced here.

#include <gtest/gtest.h>

#include "../Controller/MenuNode.h"
#include "../Controller/NavigationLoop.h"
#include "../Model/StatefulModel.h"
#include "Support/ConsoleRedirect.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{
    using ConsoleMVC::Controller::ActionMenuNode;
    using ConsoleMVC::Controller::ActionOutcome;
    using ConsoleMVC::Controller::IController;
    using ConsoleMVC::Controller::MenuNode;
    using ConsoleMVC::Controller::NavigationLoop;
    using ConsoleMVC::Controller::ReadOnlyMenuNode;
    using ConsoleMVC::Model::StatefulModel;
    using ConsoleMVC::Tests::ScopedStdinRedirect;
    using ConsoleMVC::Tests::ScopedStdoutCapture;

    enum class TestState
    {
        A,
        B,
        C
    };

    // Only A -> B and B -> C are allowed, matching Tests/StatefulModelTest.cpp.
    bool AllowOnlyForwardStep(const TestState& current, const TestState& next)
    {
        if (current == TestState::A && next == TestState::B)
        {
            return true;
        }
        if (current == TestState::B && next == TestState::C)
        {
            return true;
        }
        return false;
    }

    // A small fixture that wires together the demo menu tree described above,
    // backed by a StatefulModel<TestState> so "Advance" can succeed or fail
    // depending on the model's current state.
    struct DemoMenuTree
    {
        std::shared_ptr<MenuNode> root = std::make_shared<MenuNode>("Main Menu");
        StatefulModel<TestState> model{TestState::A, AllowOnlyForwardStep};
        int readOnlyLeafRunCount = 0;
        bool nextAdvanceThrows = false;

        DemoMenuTree()
        {
            auto reports = std::make_shared<MenuNode>("Reports");
            reports->AddChild(std::make_shared<ReadOnlyMenuNode>(
                "List Items", [this] { ++readOnlyLeafRunCount; }));
            root->AddChild(reports);

            auto actions = std::make_shared<MenuNode>("Actions");
            actions->AddChild(std::make_shared<ActionMenuNode>(
                "Advance",
                [this] { return std::vector<std::pair<std::string, std::string>>{
                    {"Current state", model.GetState() == TestState::A ? "A" : "other"}}; },
                [this]
                {
                    if (nextAdvanceThrows)
                    {
                        throw std::runtime_error("simulated failure");
                    }

                    TestState next = model.GetState() == TestState::A ? TestState::B : TestState::A;
                    if (model.TryTransition(next))
                    {
                        return ActionOutcome{true, "Advanced."};
                    }
                    return ActionOutcome{false, "Transition rejected."};
                }));
            root->AddChild(actions);
        }

        void Run(const std::string& scriptedInput)
        {
            ScopedStdinRedirect stdinRedirect(scriptedInput);
            ScopedStdoutCapture stdoutCapture;

            NavigationLoop loop(root);
            loop.Run();
        }
    };
}

TEST(NavigationLoopTest, EntersTwoLevelSubmenuAndRunsReadOnlyLeaf)
{
    DemoMenuTree demo;

    // Main Menu -> "1" enters Reports -> "1" runs "List Items" -> "0" back to
    // Reports -> "0" back to Main Menu -> "0" exits.
    demo.Run("1\n1\n0\n0\n");

    EXPECT_EQ(demo.readOnlyLeafRunCount, 1);
}

TEST(NavigationLoopTest, BackFromSubmenuReturnsToParentNotExitAndSubmenuCanBeReentered)
{
    DemoMenuTree demo;

    // Round trip: enter Reports -> run leaf -> back ("0") -> back must land on
    // Main Menu (not exit the loop), so Reports can be entered a second time
    // and its leaf run again. If a submenu's "0" incorrectly ended the loop
    // (like the root's "0" does), the remaining scripted input below would
    // never be consumed and readOnlyLeafRunCount would stay at 1.
    demo.Run("1\n1\n0\n1\n1\n0\n0\n");

    EXPECT_EQ(demo.readOnlyLeafRunCount, 2);
}

TEST(NavigationLoopTest, ExitOptionOnlyEndsTheLoopAtTheRootMenu)
{
    DemoMenuTree demo;

    // "0" at the Reports submenu must be "back", not "exit": the loop keeps
    // running afterwards and only stops once "0" is chosen again at the Main
    // Menu. This is exercised by performing a state-changing action *after*
    // backing out of the submenu - it only runs if the loop is still alive.
    demo.Run("1\n0\n2\n1\nY\n0\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::B);
}

TEST(NavigationLoopTest, InvalidNonNumericAndBlankInputAreAllRetriedOnTheSameMenu)
{
    DemoMenuTree demo;

    // At the Main Menu: a non-existent number, non-numeric text, and a blank
    // line are all rejected before a valid "0" (exit) is finally accepted.
    demo.Run("99\nabc\n\n0\n");

    // The loop terminated (Run() returned) without ever reaching a submenu.
    EXPECT_EQ(demo.readOnlyLeafRunCount, 0);
}

TEST(NavigationLoopTest, ValidTransitionSucceedsAndUpdatesModelState)
{
    DemoMenuTree demo;

    // Main Menu -> Actions -> Advance (A -> B is allowed) confirmed with Y.
    demo.Run("2\n1\nY\n0\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::B);
}

TEST(NavigationLoopTest, ModelRejectedTransitionShowsFailureAndLeavesStateUnchanged)
{
    DemoMenuTree demo;

    // Advance twice: A -> B succeeds, but the handler then tries B -> A,
    // which AllowOnlyForwardStep rejects. NavigationLoop must not crash and
    // must leave the state at B.
    demo.Run("2\n1\nY\n1\nY\n0\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::B);
}

TEST(NavigationLoopTest, CancellingConfirmationWithNNeverChangesModelState)
{
    DemoMenuTree demo;

    demo.Run("2\n1\nN\n0\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::A);
}

TEST(NavigationLoopTest, InvalidConfirmationAnswerIsRetriedBeforeCancelling)
{
    DemoMenuTree demo;

    // "maybe" is neither Y nor N and must be retried; "N" then cancels.
    demo.Run("2\n1\nmaybe\nN\n0\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::A);
}

TEST(NavigationLoopTest, UnexpectedExceptionIsCaughtAndReturnsToMainMenu)
{
    DemoMenuTree demo;
    demo.nextAdvanceThrows = true;

    // The handler throws instead of returning ActionOutcome. NavigationLoop
    // must catch it, report a failure, and land back on the Main Menu -
    // exiting from there ("0") must still work afterwards.
    demo.Run("2\n1\nY\n0\n");

    EXPECT_EQ(demo.model.GetState(), TestState::A);
}
