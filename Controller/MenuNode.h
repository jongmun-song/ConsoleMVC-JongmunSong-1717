#pragma once

// docs/feature/controller.md section 2.1 - composite menu-tree node.
//
// Three concrete IController implementations:
//   - MenuNode: a container ("submenu") that owns an ordered list of child
//     nodes. Selecting it enters it - NavigationLoop displays its children
//     next. Building a deeper menu is just nesting more MenuNode instances,
//     so tree depth is unbounded.
//   - ReadOnlyMenuNode: a leaf for query/monitoring screens. Running it has
//     no confirmation step and always reports success - it never changes
//     Model state (docs/feature/controller.md section 3.4).
//   - ActionMenuNode: a leaf for state-changing actions. It may be given a
//     confirmation-summary builder, in which case selecting it shows the
//     summary via View::ConfirmView and asks Y/N before running the handler.
//     Answering N skips the handler entirely, so a cancelled action can
//     never reach the Model (docs/feature/controller.md section 3.3,
//     "되돌아가기(N/0)는 항상 부작용 없이 취소되어야 한다"). The handler
//     itself reports success/failure via ActionOutcome, so a Model-rejected
//     transition is handled as an ordinary failure rather than an exception.
//
// Extending a menu tree with a new submenu or leaf action only requires
// constructing one of these three types and calling MenuNode::AddChild -
// NavigationLoop never needs to change (open/closed principle).

#include "IController.h"
#include "InputReader.h"
#include "../View/ConfirmView.h"
#include "../View/MessageView.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Controller
{
    // Shared by both leaf node types below: stores the label/parent and
    // reports "no children", so NavigationLoop always executes a leaf
    // instead of entering it.
    class LeafMenuNode : public IController
    {
    public:
        explicit LeafMenuNode(std::string label)
            : m_label(std::move(label))
        {
        }

        const std::string& GetLabel() const override
        {
            return m_label;
        }

        IController* GetParent() const override
        {
            return m_parent;
        }

        void SetParent(IController* parent) override
        {
            m_parent = parent;
        }

        bool HasChildren() const override
        {
            return false;
        }

        const std::vector<std::shared_ptr<IController>>& GetChildren() const override
        {
            static const std::vector<std::shared_ptr<IController>> noChildren;
            return noChildren;
        }

    private:
        std::string m_label;
        IController* m_parent = nullptr;
    };

    // A container node ("submenu"). Owns its children; NavigationLoop
    // displays them rather than calling Execute() on this node.
    class MenuNode : public IController
    {
    public:
        explicit MenuNode(std::string label)
            : m_label(std::move(label))
        {
        }

        // Appends `child` to this menu and keeps it alive for as long as
        // this menu is alive. Sets `child`'s parent to this node so
        // NavigationLoop can walk back up when the user backs out.
        void AddChild(std::shared_ptr<IController> child)
        {
            child->SetParent(this);
            m_children.push_back(std::move(child));
        }

        const std::string& GetLabel() const override
        {
            return m_label;
        }

        IController* GetParent() const override
        {
            return m_parent;
        }

        void SetParent(IController* parent) override
        {
            m_parent = parent;
        }

        bool HasChildren() const override
        {
            return !m_children.empty();
        }

        const std::vector<std::shared_ptr<IController>>& GetChildren() const override
        {
            return m_children;
        }

        // Container nodes are entered by NavigationLoop, never executed;
        // this only exists to satisfy IController.
        ActionOutcome Execute() override
        {
            return ActionOutcome{true, ""};
        }

    private:
        std::string m_label;
        IController* m_parent = nullptr;
        std::vector<std::shared_ptr<IController>> m_children;
    };

    // A read-only leaf: renders a query/monitoring screen and always
    // reports success. Has no confirmation step because it never changes
    // Model state.
    class ReadOnlyMenuNode : public LeafMenuNode
    {
    public:
        using ViewHandler = std::function<void()>;

        ReadOnlyMenuNode(std::string label, ViewHandler renderView)
            : LeafMenuNode(std::move(label))
            , m_renderView(std::move(renderView))
        {
        }

        ActionOutcome Execute() override
        {
            m_renderView();
            return ActionOutcome{true, ""};
        }

    private:
        ViewHandler m_renderView;
    };

    // A state-changing leaf.
    class ActionMenuNode : public LeafMenuNode
    {
    public:
        using Handler = std::function<ActionOutcome()>;
        using ConfirmationSummaryBuilder =
            std::function<std::vector<std::pair<std::string, std::string>>()>;

        // Runs `handler` immediately when selected, with no confirmation
        // prompt.
        ActionMenuNode(std::string label, Handler handler)
            : LeafMenuNode(std::move(label))
            , m_handler(std::move(handler))
        {
        }

        // Shows `buildSummary()` via View::ConfirmView and asks for a Y/N
        // answer before running `handler`. Answering N cancels without ever
        // calling `handler`.
        ActionMenuNode(std::string label, ConfirmationSummaryBuilder buildSummary, Handler handler)
            : LeafMenuNode(std::move(label))
            , m_buildSummary(std::move(buildSummary))
            , m_handler(std::move(handler))
        {
        }

        ActionOutcome Execute() override
        {
            if (m_buildSummary && !ConfirmedByUser())
            {
                return ActionOutcome{true, "Cancelled."};
            }

            return m_handler();
        }

    private:
        bool ConfirmedByUser() const
        {
            View::ConfirmView::Print(m_buildSummary());

            return InputReader::ReadUntilValid<bool>(
                &InputReader::ParseYesNo,
                []
                {
                    View::MessageView::Print("Please enter Y or N.", View::MessageStyle::Failure);
                });
        }

        ConfirmationSummaryBuilder m_buildSummary;
        Handler m_handler;
    };
}
