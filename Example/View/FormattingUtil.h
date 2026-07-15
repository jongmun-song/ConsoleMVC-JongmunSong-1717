#pragma once

// Shared numeric formatting helpers used by multiple Example/View adapters.
// Extracted to avoid duplicating the same std::ostringstream formatting
// logic across the screen adapters below (see docs/CODE_CONVENTION.md
// section 6 - repeated patterns are extracted once they represent the same
// concept). This header renders no screens itself - it only turns domain
// doubles into the plain strings that TableView/ConfirmView cells expect.

#include <iomanip>
#include <sstream>
#include <string>

namespace ConsoleMVC::Example::View::Detail
{
    // Formats `value` with a fixed number of decimal places, e.g.
    // FormatFixed(2.5) -> "2.50".
    inline std::string FormatFixed(double value, int precision = 2)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(precision) << value;
        return out.str();
    }

    // Renders a [0.0, 1.0] ratio as a whole-number percentage, e.g.
    // FormatPercentage(0.9) -> "90%".
    inline std::string FormatPercentage(double ratio)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(0) << (ratio * 100.0) << "%";
        return out.str();
    }
}
