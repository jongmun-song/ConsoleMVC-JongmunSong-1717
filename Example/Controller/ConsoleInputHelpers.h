#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/controller.md section 7).
//
// The example menu tree's action handlers (SampleMenuFactory.h,
// OrderMenuFactory.h, ShipmentMenuFactory.h) need to collect a few more
// input shapes than the core Controller::InputReader parses out of the box
// (it only ships ParseInt/ParseYesNo - see ../../Controller/InputReader.h).
// Rather than duplicating the "print a prompt, read a line, reject invalid
// input, ask again" loop in every handler, this header adds the two extra
// parsers this domain needs (a non-negative double, and a non-blank line)
// on top of the same shared Controller::InputReader::ReadUntilValid retry
// loop the core already uses for menu numbers and Y/N answers - so parsing/
// validation logic still lives in exactly one place
// (docs/design/phase3.md review point: "입력 파싱/검증 로직이 여러 곳에
// 중복되어 있지 않은지").
//
// This header performs console I/O (it is part of Example/Controller, the
// example domain's input-reading layer), matching
// docs/feature/controller.md section 1 ("콘솔로부터 입력을 읽는 유일한
// 계층" is Controller).

#include "../../Controller/InputReader.h"
#include "../../View/MessageView.h"

#include <cstddef>
#include <iostream>
#include <optional>
#include <string>

namespace ConsoleMVC::Example::Controller
{
    class ConsoleInputHelpers
    {
    public:
        // Prints `prompt`, then reads lines until one is non-blank.
        static std::string ReadNonEmptyLine(const std::string& prompt)
        {
            std::cout << prompt;
            return ConsoleMVC::Controller::InputReader::ReadUntilValid<std::string>(
                &ParseNonEmptyLine,
                [] { ConsoleMVC::View::MessageView::Print("Input must not be blank.", ConsoleMVC::View::MessageStyle::Failure); });
        }

        // Prints `prompt`, then reads lines until one parses as an integer
        // (reuses Controller::InputReader::ParseInt - the same parser menu
        // number selection uses).
        static int ReadInt(const std::string& prompt)
        {
            std::cout << prompt;
            return ConsoleMVC::Controller::InputReader::ReadUntilValid<int>(
                &ConsoleMVC::Controller::InputReader::ParseInt,
                [] { ConsoleMVC::View::MessageView::Print("Please enter a whole number.", ConsoleMVC::View::MessageStyle::Failure); });
        }

        // Prints `prompt`, then reads lines until one parses as a
        // non-negative double.
        static double ReadNonNegativeDouble(const std::string& prompt)
        {
            std::cout << prompt;
            return ConsoleMVC::Controller::InputReader::ReadUntilValid<double>(
                &ParseNonNegativeDouble,
                [] { ConsoleMVC::View::MessageView::Print("Please enter a non-negative number.", ConsoleMVC::View::MessageStyle::Failure); });
        }

        // Reads lines until one parses as Y/N, without printing a prompt
        // first - for callers whose confirmation view (e.g.
        // SampleRegistrationView::PrintConfirmation) already printed the
        // "Y/N > " prompt as the last line of its own output.
        static bool ReadYesNo()
        {
            return ConsoleMVC::Controller::InputReader::ReadUntilValid<bool>(
                &ConsoleMVC::Controller::InputReader::ParseYesNo,
                [] { ConsoleMVC::View::MessageView::Print("Please enter Y or N.", ConsoleMVC::View::MessageStyle::Failure); });
        }

        // Prints `prompt`, then reads lines until one parses as Y/N (reuses
        // Controller::InputReader::ParseYesNo - the same parser
        // ActionMenuNode's built-in confirmation step uses). For callers
        // that have not already printed a confirmation view of their own.
        static bool ReadYesNo(const std::string& prompt)
        {
            std::cout << prompt;
            return ReadYesNo();
        }

    private:
        static std::optional<std::string> ParseNonEmptyLine(const std::string& text)
        {
            if (text.empty())
            {
                return std::nullopt;
            }
            return text;
        }

        static std::optional<double> ParseNonNegativeDouble(const std::string& text)
        {
            if (text.empty())
            {
                return std::nullopt;
            }

            try
            {
                std::size_t charsConsumed = 0;
                const double value = std::stod(text, &charsConsumed);
                if (charsConsumed != text.size() || value < 0.0)
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
    };
}
