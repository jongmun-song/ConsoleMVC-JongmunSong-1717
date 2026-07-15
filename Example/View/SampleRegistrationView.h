#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/view.md section 7).
//
// SampleRegistrationView adapts a not-yet-persisted Example::Model::Sample
// into the key-value summary shape ConsoleMVC::View::ConfirmView renders,
// asking the caller to confirm registration before it is added to storage.

#include "../Model/Sample.h"
#include "FormattingUtil.h"
#include "../../View/ConfirmView.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::View
{
    class SampleRegistrationView
    {
    public:
        static std::string RenderConfirmation(
            const Model::Sample& sample,
            const std::string& prompt = "Register this sample? (Y/N) > ")
        {
            return ConsoleMVC::View::ConfirmView::Render(ToSummary(sample), prompt);
        }

        static void PrintConfirmation(
            const Model::Sample& sample,
            const std::string& prompt = "Register this sample? (Y/N) > ")
        {
            std::cout << RenderConfirmation(sample, prompt);
        }

    private:
        static std::vector<std::pair<std::string, std::string>> ToSummary(const Model::Sample& sample)
        {
            return {
                { "ID", std::to_string(sample.GetId()) },
                { "Name", sample.GetName() },
                { "Avg. Production Time", Detail::FormatFixed(sample.GetAverageProductionTimePerUnit()) },
                { "Yield Ratio", Detail::FormatPercentage(sample.GetYieldRatio()) },
                { "Initial Stock", std::to_string(sample.GetStockQuantity()) }
            };
        }
    };
}
