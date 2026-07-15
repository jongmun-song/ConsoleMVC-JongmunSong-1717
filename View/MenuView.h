#pragma once

// docs/feature/view.md section 3.2 - numbered-menu rendering
//
// MenuView renders a list of { number, label } pairs as a numbered menu.
// It does not reserve or special-case any particular number (e.g. 0 for
// "back"/"exit") - that convention belongs to the caller (Controller/
// MenuNode), which decides both which numbers to use and what their
// labels say.

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ConsoleMVC::View
{
    struct MenuItem
    {
        int number;
        std::string label;
    };

    class MenuView
    {
    public:
        // Renders `items` in the order given, one "[number] label" line
        // each, followed by `prompt` on its own line (no trailing newline
        // after the prompt, so a Controller can read input right after it).
        static std::string Render(const std::vector<MenuItem>& items, const std::string& prompt = "> ")
        {
            std::ostringstream out;
            for (const auto& item : items)
            {
                out << "[" << item.number << "] " << item.label << "\n";
            }
            out << prompt;
            return out.str();
        }

        static void Print(const std::vector<MenuItem>& items, const std::string& prompt = "> ")
        {
            std::cout << Render(items, prompt);
        }
    };
}
