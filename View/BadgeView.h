#pragma once

// docs/feature/view.md section 3.4 - badge/label rendering
//
// BadgeView wraps an arbitrary label string in a visual emphasis marker.
// BadgeStyle is a generic emphasis category (Normal/Warning/Error), not a
// domain state enumeration - callers map their own domain states (e.g.
// order status) to one of these categories before calling Render().

#include <iostream>
#include <string>

namespace ConsoleMVC::View
{
    enum class BadgeStyle
    {
        Normal,
        Warning,
        Error
    };

    class BadgeView
    {
    public:
        // Renders `label` wrapped in brackets, with a style marker prefix
        // that depends on `style`. The label text itself is never
        // interpreted - it is displayed verbatim.
        static std::string Render(const std::string& label, BadgeStyle style = BadgeStyle::Normal)
        {
            return "[" + StyleMarker(style) + label + "]";
        }

        static void Print(const std::string& label, BadgeStyle style = BadgeStyle::Normal)
        {
            std::cout << Render(label, style);
        }

    private:
        static std::string StyleMarker(BadgeStyle style)
        {
            switch (style)
            {
            case BadgeStyle::Warning:
                return "! ";
            case BadgeStyle::Error:
                return "!! ";
            case BadgeStyle::Normal:
            default:
                return "";
            }
        }
    };
}
