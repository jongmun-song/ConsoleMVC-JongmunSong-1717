// Sanity check for the gtest pipeline itself (Phase 0 scope).
//
// This test does not exercise any Model/View/Controller behavior yet -
// those tests arrive alongside their respective Phase 1-3 implementations.
// Its only purpose is to prove that packages/gtest.1.7.0 links and runs
// correctly from this project (see main.cpp for the CONSOLEMVC_RUN_TESTS
// entry point that drives RUN_ALL_TESTS()).

#include <gtest/gtest.h>

TEST(Sanity, AlwaysTrue)
{
    EXPECT_TRUE(true);
}
