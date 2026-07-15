#pragma once

// docs/feature/controller.md section 2.1 - composite menu-tree contract.
//
// IController is the common interface implemented by every node in a menu
// tree: container nodes ("submenus", see MenuNode) that own child nodes and
// leaf nodes (see ReadOnlyMenuNode / ActionMenuNode in MenuNode.h) that
// perform work when selected. NavigationLoop drives the whole tree purely
// through this interface, so adding a new submenu or leaf action anywhere in
// the tree never requires changing NavigationLoop itself (open/closed
// principle).

#include <memory>
#include <string>
#include <vector>

namespace ConsoleMVC::Controller
{
    // Result of executing a leaf node (ReadOnlyMenuNode::Execute /
    // ActionMenuNode::Execute). NavigationLoop shows `message` via
    // View::MessageView (styled by `succeeded`) when it is non-empty.
    //
    // This is how a Model-level rejected state transition is surfaced:
    // docs/feature/controller.md section 4 treats
    // "Model 계층에서 유효하지 않은 상태 전이를 거부한 경우" as an ordinary
    // failed outcome, not an exception - a leaf's handler calls something
    // like StatefulModel::TryTransition and translates a `false` result into
    // ActionOutcome{false, "reason"} instead of throwing.
    struct ActionOutcome
    {
        bool succeeded = true;
        std::string message;
    };

    class IController
    {
    public:
        virtual ~IController() = default;

        // Label shown for this node in its parent's numbered menu.
        virtual const std::string& GetLabel() const = 0;

        // The menu this node belongs to, or nullptr for the top-level menu.
        // NavigationLoop uses this both to detect the root (which shows
        // "Exit" instead of "Back") and to walk back up when the user backs
        // out of a submenu.
        virtual IController* GetParent() const = 0;
        virtual void SetParent(IController* parent) = 0;

        // True for container nodes whose children should be displayed when
        // this node is selected (NavigationLoop "enters" it). False for leaf
        // nodes, which NavigationLoop executes immediately instead.
        virtual bool HasChildren() const = 0;
        virtual const std::vector<std::shared_ptr<IController>>& GetChildren() const = 0;

        // Runs this node's action. Only ever called on leaf nodes
        // (HasChildren() == false) - container nodes are entered rather than
        // executed, and their Execute() is a harmless no-op.
        virtual ActionOutcome Execute() = 0;
    };
}
