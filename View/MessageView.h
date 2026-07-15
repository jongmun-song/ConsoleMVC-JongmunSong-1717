#pragma once

// docs/feature/view.md section 3.7 - result/status message rendering
//
// MessageView renders a free-form message string with a style-specific
// prefix. MessageStyle is a generic classification (Success/Failure/Info),
// not a domain-specific result code - the message text itself is supplied
// entirely by the caller.

#include <iostream>
#include <string>

namespace ConsoleMVC::View
{
    enum class MessageStyle
    {
        Success,
        Failure,
        Info
    };

    class MessageView
    {
    public:
        static std::string Render(const std::string& message, MessageStyle style = MessageStyle::Info)
        {
            return Prefix(style) + message;
        }

        static void Print(const std::string& message, MessageStyle style = MessageStyle::Info)
        {
            std::cout << Render(message, style) << "\n";
        }

    private:
        static std::string Prefix(MessageStyle style)
        {
            switch (style)
            {
            case MessageStyle::Success:
                return "[OK] ";
            case MessageStyle::Failure:
                return "[FAIL] ";
            case MessageStyle::Info:
            default:
                return "[INFO] ";
            }
        }
    };
}
