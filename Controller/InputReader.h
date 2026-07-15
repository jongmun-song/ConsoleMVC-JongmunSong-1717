#pragma once

// docs/feature/controller.md section 2.3 - console input utility.
//
// InputReader is the only place in this skeleton that reads from std::cin
// (docs/feature/controller.md section 1, "콘솔로부터 입력을 읽는 유일한
// 계층"). It provides:
//   - ReadLine/Trim: reading and normalizing one line of input
//   - ParseInt/ParseYesNo: the two parsers every menu screen needs (a menu
//     number, or a Y/N confirmation answer)
//   - ReadUntilValid: the "keep asking until the input parses" retry loop
//     shared by both of the above, so NavigationLoop and ActionMenuNode do
//     not each re-implement it (docs/feature/controller.md section 4 -
//     invalid/blank input is always handled by re-prompting, never by
//     crashing or exiting).

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <optional>
#include <string>

namespace ConsoleMVC::Controller
{
    class InputReader
    {
    public:
        // Reads one line from std::cin and trims surrounding whitespace.
        // If the stream has no more input (e.g. piped input exhausted), the
        // failure is cleared and an empty string is returned so callers can
        // treat it the same as a blank line rather than crashing.
        static std::string ReadLine()
        {
            std::string line;
            if (!std::getline(std::cin, line))
            {
                std::cin.clear();
                return "";
            }
            return Trim(line);
        }

        // Removes leading/trailing whitespace from `text`.
        static std::string Trim(const std::string& text)
        {
            const auto isSpace = [](unsigned char character) { return std::isspace(character) != 0; };

            const auto begin = std::find_if_not(text.begin(), text.end(), isSpace);
            const auto end = std::find_if_not(text.rbegin(), text.rend(), isSpace).base();

            if (begin >= end)
            {
                return "";
            }
            return std::string(begin, end);
        }

        // Parses `text` as a base-10 integer. Returns std::nullopt for blank
        // input, non-numeric input, and input that is only partially
        // numeric ("12abc") - a menu number must be the entire line.
        static std::optional<int> ParseInt(const std::string& text)
        {
            if (text.empty())
            {
                return std::nullopt;
            }

            try
            {
                std::size_t charsConsumed = 0;
                const int value = std::stoi(text, &charsConsumed);
                if (charsConsumed != text.size())
                {
                    return std::nullopt;
                }
                return value;
            }
            catch (const std::exception&)
            {
                return std::nullopt;
            }
        }

        // Parses `text` as a case-insensitive Y/N answer. Returns
        // std::nullopt for anything else, including blank input.
        static std::optional<bool> ParseYesNo(const std::string& text)
        {
            if (text.size() != 1)
            {
                return std::nullopt;
            }

            switch (std::tolower(static_cast<unsigned char>(text[0])))
            {
            case 'y':
                return true;
            case 'n':
                return false;
            default:
                return std::nullopt;
            }
        }

        // Reads lines and applies `parse` to each until it returns a value,
        // invoking `onInvalidInput` between attempts (typically to show a
        // failure message and/or re-issue the prompt). Centralizes the
        // "reject invalid input, ask again" loop so every place that needs
        // typed console input - menu numbers, Y/N confirmations, and any
        // future ones - shares a single implementation.
        template <typename TValue>
        static TValue ReadUntilValid(
            const std::function<std::optional<TValue>(const std::string&)>& parse,
            const std::function<void()>& onInvalidInput)
        {
            while (true)
            {
                const std::string line = ReadLine();
                if (std::optional<TValue> parsed = parse(line))
                {
                    return *parsed;
                }
                onInvalidInput();
            }
        }
    };
}
