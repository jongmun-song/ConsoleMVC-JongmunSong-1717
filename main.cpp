// ConsoleMVC entry point.
//
// Phase 4 scope (docs/design/phase4.md item 1, "통합 와이어링"): the console
// demo below builds ConsoleMVC::Example::Controller::ExampleMenuTreeFactory's
// full example menu tree (Sample Management / Place a new order / Order
// Approval-Rejection / Monitoring / Production Line / Release a shipment)
// on top of an ExampleAppContext, and drives it with the core
// Controller::NavigationLoop. This is the same wiring pattern any consumer
// of this skeleton (most notably SampleOrderSystem) is expected to follow:
// build a domain AppContext, build a menu tree out of MenuNode/
// ActionMenuNode/ReadOnlyMenuNode, and hand it to NavigationLoop.
//
// IMPORTANT: everything under Example/ (including ExampleAppContext and the
// menu tree built here) is a PoC verification example, not reusable
// application code - see ../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)".
// SampleOrderSystem is expected to define its own domain Model/View/
// Controller (its own AppContext, its own menu factories) on top of the core
// Model/View/Controller contracts; it is not expected to depend on anything
// under Example/. ExampleAppContext keeps all data in memory only - it is
// never given a persistence adapter, and the process's data disappears when
// it exits (docs/PRD.md section 3 - no persistence in this repository).
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

#include "Controller/NavigationLoop.h"
#include "Example/Controller/ExampleAppContext.h"
#include "Example/Controller/ExampleMenuTreeFactory.h"

#include <Windows.h>

int main()
{
    // Keep Korean console output/input working for both this file's own
    // messages and every Example/View screen the menu tree below renders
    // (see the note this replaced for why UTF-8 is used instead of the
    // legacy Korean code page).
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "안녕하세요, ConsoleMVC 예시 도메인 데모입니다." << std::endl;
    std::cout << "(시료 등록 -> 주문 접수 -> 승인/거절 -> 생산 -> 출고까지의 흐름을 확인할 수 있습니다.)"
              << std::endl;

    // context must outlive the menu tree/NavigationLoop below - every leaf's
    // handler captures it by reference (see ExampleMenuTreeFactory.h).
    ConsoleMVC::Example::Controller::ExampleAppContext context;

    ConsoleMVC::Controller::NavigationLoop navigationLoop(
        ConsoleMVC::Example::Controller::ExampleMenuTreeFactory::BuildMainMenu(context));
    navigationLoop.Run();

    return 0;
}

#endif
