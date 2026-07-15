#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7 - "시료 관리").
//
// Builds the "시료 관리" submenu: a ReadOnlyMenuNode listing every Sample
// (Example::View::SampleListView) and an ActionMenuNode that registers a new
// one (Example::View::SampleRegistrationView).
//
// The registration handler does not use ActionMenuNode's built-in
// buildSummary+confirm constructor: that confirmation step only shows a
// summary computed with no arguments, but here the summary depends on the
// name/time/yield/stock the user is asked to type in first. So this handler
// collects those fields itself, renders the same confirmation view, and
// asks Y/N via the same Controller::InputReader-backed helper
// (ConsoleInputHelpers::ReadYesNo) ActionMenuNode uses internally - only
// inlined because the entity to confirm is not known until after input is
// collected. Answering N returns before ExampleAppContext::sampleModel is
// touched, so cancelling still has no side effects.

#include "ConsoleInputHelpers.h"
#include "ExampleAppContext.h"
#include "../Model/Sample.h"
#include "../View/SampleListView.h"
#include "../View/SampleRegistrationView.h"
#include "../../Controller/MenuNode.h"

#include <memory>
#include <string>

namespace ConsoleMVC::Example::Controller
{
    class SampleMenuFactory
    {
    public:
        static std::shared_ptr<ConsoleMVC::Controller::IController> Build(ExampleAppContext& context)
        {
            auto menu = std::make_shared<ConsoleMVC::Controller::MenuNode>("Sample Management");

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ReadOnlyMenuNode>(
                "List samples",
                [&context] { View::SampleListView::Print(context.sampleModel.GetAll()); }));

            menu->AddChild(std::make_shared<ConsoleMVC::Controller::ActionMenuNode>(
                "Register a new sample",
                [&context] { return RegisterSample(context); }));

            return menu;
        }

    private:
        static ConsoleMVC::Controller::ActionOutcome RegisterSample(ExampleAppContext& context)
        {
            const std::string name = ConsoleInputHelpers::ReadNonEmptyLine("Sample name > ");
            const double averageProductionTimePerUnit =
                ConsoleInputHelpers::ReadNonNegativeDouble("Average production time per unit > ");
            const double yieldRatio = ConsoleInputHelpers::ReadNonNegativeDouble("Yield ratio (0.0 - 1.0) > ");
            const int initialStockQuantity = ConsoleInputHelpers::ReadInt("Initial stock quantity > ");

            const Model::Sample candidate(
                context.nextSampleId, name, averageProductionTimePerUnit, yieldRatio, initialStockQuantity);

            View::SampleRegistrationView::PrintConfirmation(candidate);
            if (!ConsoleInputHelpers::ReadYesNo())
            {
                return {true, "Cancelled."};
            }

            context.sampleModel.Add(candidate);
            context.TakeNextSampleId();
            return {true, "Sample registered."};
        }
    };
}
