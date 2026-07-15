// Phase 1-B - StatefulModel<TState> contract tests.
//
// docs/design/phase1.md "테스트 포인트 (tester)" section for the 1-B scope:
//   - Valid transition: TryTransition returns true and GetState reflects the
//     new state
//   - Invalid transition (validator rejects it): TryTransition returns false
//     and GetState is unchanged (state invariant)
//
// The state type used here is a generic placeholder enum (TestState) - no
// domain-specific state names (e.g. RESERVED/CONFIRMED) are introduced, per
// this repository's domain-agnostic constraint.

#include <gtest/gtest.h>

#include "../Model/StatefulModel.h"

namespace
{
    using ConsoleMVC::Model::StatefulModel;

    enum class TestState
    {
        A,
        B,
        C
    };

    // Only allows A -> B and B -> C. Every other transition (including
    // staying in place, or going backwards) is rejected.
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
}

TEST(StatefulModelTest, InitialStateMatchesConstructorArgument)
{
    StatefulModel<TestState> model(TestState::A, AllowOnlyForwardStep);

    EXPECT_EQ(model.GetState(), TestState::A);
}

TEST(StatefulModelTest, ValidTransitionSucceedsAndUpdatesState)
{
    StatefulModel<TestState> model(TestState::A, AllowOnlyForwardStep);

    const bool result = model.TryTransition(TestState::B);

    EXPECT_TRUE(result);
    EXPECT_EQ(model.GetState(), TestState::B);
}

TEST(StatefulModelTest, InvalidTransitionFailsAndLeavesStateUnchanged)
{
    StatefulModel<TestState> model(TestState::A, AllowOnlyForwardStep);

    // A -> C is not an allowed step (only A -> B and B -> C are).
    const bool result = model.TryTransition(TestState::C);

    EXPECT_FALSE(result);
    EXPECT_EQ(model.GetState(), TestState::A);
}

TEST(StatefulModelTest, TransitionBackwardsIsRejected)
{
    StatefulModel<TestState> model(TestState::A, AllowOnlyForwardStep);
    ASSERT_TRUE(model.TryTransition(TestState::B));

    const bool result = model.TryTransition(TestState::A);

    EXPECT_FALSE(result);
    EXPECT_EQ(model.GetState(), TestState::B);
}

TEST(StatefulModelTest, MultipleValidTransitionsChainCorrectly)
{
    StatefulModel<TestState> model(TestState::A, AllowOnlyForwardStep);

    ASSERT_TRUE(model.TryTransition(TestState::B));
    ASSERT_TRUE(model.TryTransition(TestState::C));

    EXPECT_EQ(model.GetState(), TestState::C);
}

TEST(StatefulModelTest, ValidatorReceivesCurrentAndNextState)
{
    TestState observedCurrent = TestState::C;
    TestState observedNext = TestState::C;

    StatefulModel<TestState> model(
        TestState::A,
        [&observedCurrent, &observedNext](const TestState& current, const TestState& next)
        {
            observedCurrent = current;
            observedNext = next;
            return true;
        });

    model.TryTransition(TestState::B);

    EXPECT_EQ(observedCurrent, TestState::A);
    EXPECT_EQ(observedNext, TestState::B);
}
