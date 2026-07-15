// ConsoleMVC entry point.
//
// Phase 0 scope: this file only proves that the console can display Korean
// text correctly. Wiring Model/View/Controller together is Phase 4's job.
//
// NOTE: This file is saved as UTF-8 with BOM so that MSVC compiles the
// Korean string literals below correctly. Setting the console output/input
// code page to UTF-8 (CP_UTF8 = 65001) makes `std::cout` print Korean text
// without garbling on modern Windows terminals. Legacy cmd.exe consoles
// that still use a raster font may not render Hangul glyphs even with the
// code page fixed; switching the console font to a TrueType font (e.g.
// "Consolas" or "Malgun Gothic") resolves that separately.
//
// This translation unit normally builds the console demo below. When the
// project is built with ConsoleMVCRunTests=1 (see ConsoleMVC.vcxproj), this
// file instead hosts the gtest entry point so the same executable can run
// the test suite (see Tests/SanityTest.cpp) without needing a second
// project.

#include <iostream>

#if CONSOLEMVC_RUN_TESTS

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#else

#include <Windows.h>

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "안녕하세요, ConsoleMVC 골격입니다." << std::endl;

    return 0;
}

#endif
