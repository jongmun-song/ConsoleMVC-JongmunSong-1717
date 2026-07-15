#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "PoC 검증용 예시 메뉴 트리").
//
// Assembles the full example menu tree out of the per-section factories
// (SampleMenuFactory, OrderMenuFactory, MonitoringMenuFactory,
// ProductionMenuFactory, ShipmentMenuFactory), all built purely from the
// core Controller::MenuNode/ReadOnlyMenuNode/ActionMenuNode/IController
// contracts (Controller/ core is not modified anywhere in Example/):
//
//   Main Menu
//    +- Sample Management
//    |   +- List samples             (ReadOnlyMenuNode)
//    |   +- Register a new sample    (ActionMenuNode)
//    +- Place a new order            (ActionMenuNode)
//    +- Order Approval / Rejection
//    |   +- Approve a reserved order (ActionMenuNode)
//    |   +- Reject a reserved order  (ActionMenuNode)
//    +- Monitoring                   (ReadOnlyMenuNode)
//    +- Production Line
//    |   +- Show production queue    (ReadOnlyMenuNode)
//    |   +- Advance production line  (ActionMenuNode - see
//    |                                 ProductionMenuFactory.h for why this
//    |                                 extra item was added)
//    +- Release a shipment           (ActionMenuNode)
//
// Run this tree with Controller::NavigationLoop(BuildMainMenu(context)).Run();
// `context` must outlive the returned tree and the NavigationLoop that runs
// it, since every leaf's handler captures it by reference.

#include "ExampleAppContext.h"
#include "MonitoringMenuFactory.h"
#include "OrderMenuFactory.h"
#include "ProductionMenuFactory.h"
#include "SampleMenuFactory.h"
#include "ShipmentMenuFactory.h"
#include "../../Controller/MenuNode.h"

#include <memory>

namespace ConsoleMVC::Example::Controller
{
    class ExampleMenuTreeFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> BuildMainMenu(ExampleAppContext& context)
        {
            auto mainMenu = std::make_shared<ConsoleMVC::Controller::MenuNode>("Main Menu");

            mainMenu->AddChild(SampleMenuFactory::Build(context));
            mainMenu->AddChild(OrderMenuFactory::BuildIntakeAction(context));
            mainMenu->AddChild(OrderMenuFactory::BuildApprovalMenu(context));
            mainMenu->AddChild(MonitoringMenuFactory::Build(context));
            mainMenu->AddChild(ProductionMenuFactory::Build(context));
            mainMenu->AddChild(ShipmentMenuFactory::Build(context));

            return mainMenu;
        }
    };
}
