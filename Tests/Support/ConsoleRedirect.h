#pragma once

// Test-only helpers for exercising Controller code that reads std::cin and
// writes std::cout directly (InputReader/NavigationLoop/MenuNode). Not part
// of the ConsoleMVC skeleton itself - used only by Tests/*.

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

namespace ConsoleMVC::Tests
{
    // Temporarily redirects std::cin to read from `scriptedInput`, restoring
    // the original buffer when this object goes out of scope.
    class ScopedStdinRedirect
    {
    public:
        explicit ScopedStdinRedirect(const std::string& scriptedInput)
            : m_input(scriptedInput)
            , m_originalBuffer(std::cin.rdbuf(m_input.rdbuf()))
        {
        }

        ~ScopedStdinRedirect()
        {
            std::cin.rdbuf(m_originalBuffer);
        }

    private:
        std::istringstream m_input;
        std::streambuf* m_originalBuffer;
    };

    // Temporarily redirects std::cout into an in-memory buffer (so tests do
    // not spam the console and can assert on what was printed), restoring
    // the original buffer when this object goes out of scope.
    class ScopedStdoutCapture
    {
    public:
        ScopedStdoutCapture()
            : m_originalBuffer(std::cout.rdbuf(m_captured.rdbuf()))
        {
        }

        ~ScopedStdoutCapture()
        {
            std::cout.rdbuf(m_originalBuffer);
        }

        std::string Str() const
        {
            return m_captured.str();
        }

    private:
        std::ostringstream m_captured;
        std::streambuf* m_originalBuffer;
    };
}
