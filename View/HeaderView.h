#pragma once

// docs/feature/view.md section 3.1 - screen header rendering
//
// HeaderView renders a screen title plus an optional, externally supplied
// piece of context (e.g. a timestamp). It never reads the clock itself -
// the caller decides what "subtitle" means and passes it in, so HeaderView
// stays free of any notion of time or domain state.

#include <iostream>
#include <sstream>
#include <string>

namespace ConsoleMVC::View
{
    class HeaderView
    {
    public:
        // Renders `title` as a screen header. `subtitle` is optional,
        // caller-supplied context (timestamp, page indicator, etc.) shown
        // next to the title; an empty string omits it entirely.
        static std::string Render(const std::string& title, const std::string& subtitle = "")
        {
            std::ostringstream out;
            out << "== " << title;
            if (!subtitle.empty())
            {
                out << "  " << subtitle;
            }
            out << " ==\n";
            return out.str();
        }

        // Thin wrapper around Render() for callers that just want the
        // header written to the console immediately.
        static void Print(const std::string& title, const std::string& subtitle = "")
        {
            std::cout << Render(title, subtitle);
        }
    };
}
