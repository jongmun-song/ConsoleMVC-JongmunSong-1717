#pragma once

// docs/feature/controller.md section 2.2 - render/read/validate/dispatch loop.
//
// NavigationLoop drives an IController tree purely through the IController
// interface: render the current node's children as a numbered menu
// (View::MenuView), read+validate one selection (InputReader), and move to
// either the selected child (a container is entered), the parent (back), the
// same node again (invalid input, or a leaf just finished running), or
// nothing (the root's "Exit" option was chosen). Because it never inspects
// concrete node types, adding new submenus/leaves anywhere in the tree never
// requires editing this file (open/closed principle).

#include "IController.h"
#include "InputReader.h"
#include "../View/MenuView.h"
#include "../View/MessageView.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Controller
{
    class NavigationLoop
    {
    public:
        // `rootMenu` must be the top-level menu (GetParent() == nullptr).
        // `backOrExitNumber` is the menu number reserved for "back"/"exit"
        // (0 by convention - docs/feature/controller.md section 3.1).
        explicit NavigationLoop(std::shared_ptr<IController> rootMenu, int backOrExitNumber = 0)
            : m_root(std::move(rootMenu))
            , m_backOrExitNumber(backOrExitNumber)
        {
        }

        // Runs screens until the root menu's back/exit option is chosen.
        // Any exception raised while rendering/executing a screen is caught
        // here, shown as a failure message, and followed by a return to the
        // root menu (docs/feature/controller.md section 4, "예기치 않은
        // 런타임 예외") - it never propagates out of Run() and crashes the
        // application.
        void Run()
        {
            IController* current = m_root.get();

            while (current != nullptr)
            {
                try
                {
                    current = RunOneMenuScreen(current);
                }
                catch (const std::exception& error)
                {
                    View::MessageView::Print(
                        std::string("Unexpected error: ") + error.what(), View::MessageStyle::Failure);
                    current = m_root.get();
                }
                catch (...)
                {
                    View::MessageView::Print("Unexpected error.", View::MessageStyle::Failure);
                    current = m_root.get();
                }
            }
        }

    private:
        // Renders `menu`'s children, reads one validated selection, and
        // returns the node NavigationLoop should show next.
        IController* RunOneMenuScreen(IController* menu)
        {
            const std::vector<std::shared_ptr<IController>>& children = menu->GetChildren();
            const bool isRootMenu = (menu->GetParent() == nullptr);

            RenderMenu(children, isRootMenu);

            const int selection = InputReader::ReadUntilValid<int>(
                &InputReader::ParseInt,
                []
                {
                    View::MessageView::Print("Please enter a number.", View::MessageStyle::Failure);
                });

            if (selection == m_backOrExitNumber)
            {
                return isRootMenu ? nullptr : menu->GetParent();
            }

            IController* selectedChild = FindChildByNumber(children, selection);
            if (selectedChild == nullptr)
            {
                View::MessageView::Print("No such menu item.", View::MessageStyle::Failure);
                return menu;
            }

            if (selectedChild->HasChildren())
            {
                return selectedChild;
            }

            ShowOutcome(selectedChild->Execute());
            return menu;
        }

        void RenderMenu(const std::vector<std::shared_ptr<IController>>& children, bool isRootMenu) const
        {
            std::vector<View::MenuItem> items;
            items.reserve(children.size() + 1);
            for (std::size_t index = 0; index < children.size(); ++index)
            {
                items.push_back(View::MenuItem{static_cast<int>(index + 1), children[index]->GetLabel()});
            }
            items.push_back(View::MenuItem{m_backOrExitNumber, isRootMenu ? "Exit" : "Back"});

            View::MenuView::Print(items);
        }

        static IController* FindChildByNumber(
            const std::vector<std::shared_ptr<IController>>& children, int selection)
        {
            if (selection < 1 || static_cast<std::size_t>(selection) > children.size())
            {
                return nullptr;
            }
            return children[static_cast<std::size_t>(selection) - 1].get();
        }

        static void ShowOutcome(const ActionOutcome& outcome)
        {
            if (outcome.message.empty())
            {
                return;
            }
            View::MessageView::Print(
                outcome.message, outcome.succeeded ? View::MessageStyle::Success : View::MessageStyle::Failure);
        }

        std::shared_ptr<IController> m_root;
        int m_backOrExitNumber;
    };
}
