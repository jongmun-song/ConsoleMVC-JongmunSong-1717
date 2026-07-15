#pragma once

// docs/feature/model.md section 3.3 - Stateful Model extension point
//
// StatefulModel<TState> generalizes "an object that holds a current state
// and moves between a finite set of states" without knowing what those
// states actually mean. Concrete domain layers (e.g. SampleOrderSystem)
// supply:
//   - the state type TState (an enum, a string, whatever fits the domain)
//   - a transition validator: a predicate that decides whether moving from
//     one state to another is allowed
//
// The validator is injected as a std::function rather than requiring
// subclassing, so that simple domains can pass a lambda/table lookup
// without introducing a new class per state machine. This keeps the
// skeleton usable both by inheritance (a subclass can build the
// std::function from its own virtual rules) and by composition (a caller
// wires up the function directly).
//
// TryTransition never throws: an invalid transition is a normal, expected
// outcome (see docs/feature/controller.md section 4 - "Model-level invalid
// transitions are handled as an ordinary failure, not an exception").

#include <functional>

namespace ConsoleMVC::Model
{
    template <typename TState>
    class StatefulModel
    {
    public:
        // Decides whether a transition from `current` to `next` is allowed.
        using TransitionValidator =
            std::function<bool(const TState& current, const TState& next)>;

        StatefulModel(TState initialState, TransitionValidator isTransitionAllowed)
            : m_currentState(std::move(initialState))
            , m_isTransitionAllowed(std::move(isTransitionAllowed))
        {
        }

        virtual ~StatefulModel() = default;

        const TState& GetState() const
        {
            return m_currentState;
        }

        // Attempts to move to `nextState`. Returns true and updates the
        // current state if the injected validator allows the transition;
        // returns false and leaves the state unchanged otherwise. Callers
        // (Controller) treat a false result as an ordinary failure, not an
        // exceptional one.
        bool TryTransition(const TState& nextState)
        {
            if (!m_isTransitionAllowed(m_currentState, nextState))
            {
                return false;
            }

            m_currentState = nextState;
            return true;
        }

    private:
        TState m_currentState;
        TransitionValidator m_isTransitionAllowed;
    };
}
