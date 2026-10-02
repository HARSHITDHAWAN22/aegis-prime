#include <gtest/gtest.h>
#include "aegis/kv/store.h"

// Confirms the build + test pipeline works end-to-end before any real
// engine code exists.
TEST(Smoke, BuildPipelineWorks) {
    EXPECT_EQ(1 + 1, 2);
}