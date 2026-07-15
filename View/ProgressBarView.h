#pragma once

// docs/feature/view.md section 3.6 - progress/ratio rendering
//
// ProgressBarView turns a 0.0-1.0 ratio into a fixed-width bar graph plus a
// percentage label. Ratios outside [0.0, 1.0] are clamped rather than
// rejected, since a caller-computed ratio (e.g. produced/ordered) can
// transiently overshoot 1.0 or dip below 0.0 depending on domain rounding;
// clamping keeps rendering total and side-effect free.

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace ConsoleMVC::View
{
    class ProgressBarView
    {
    public:
        // Renders `ratio` (clamped to [0.0, 1.0]) as a `barWidth`-character
        // bar ("#" for filled, "-" for empty) followed by a rounded
        // percentage, e.g. "[####------] 40%".
        static std::string Render(double ratio, int barWidth = 20)
        {
            const double clampedRatio = std::clamp(ratio, 0.0, 1.0);
            const int filledWidth = static_cast<int>(clampedRatio * barWidth + 0.5);
            const int percent = static_cast<int>(clampedRatio * 100.0 + 0.5);

            std::ostringstream out;
            out << "[";
            for (int i = 0; i < barWidth; ++i)
            {
                out << (i < filledWidth ? '#' : '-');
            }
            out << "] " << percent << "%";
            return out.str();
        }

        static void Print(double ratio, int barWidth = 20)
        {
            std::cout << Render(ratio, barWidth) << "\n";
        }
    };
}
