#pragma once

// docs/feature/view.md section 3.5 - confirmation summary rendering
//
// ConfirmView renders a key-value summary followed by a Y/N prompt phrase.
// It only produces the phrase asking for confirmation - reading the actual
// Y/N answer from the console is the Controller's job (see controller.md
// section 4 and view.md section 4).

#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::View
{
    class ConfirmView
    {
    public:
        // Renders `summary` as one "key: value" line per entry, in order,
        // followed by `prompt` on its own line (no trailing newline after
        // the prompt, so a Controller can read input right after it).
        static std::string Render(
            const std::vector<std::pair<std::string, std::string>>& summary,
            const std::string& prompt = "Y/N > ")
        {
            std::ostringstream out;
            for (const auto& [key, value] : summary)
            {
                out << key << ": " << value << "\n";
            }
            out << prompt;
            return out.str();
        }

        static void Print(
            const std::vector<std::pair<std::string, std::string>>& summary,
            const std::string& prompt = "Y/N > ")
        {
            std::cout << Render(summary, prompt);
        }
    };
}
