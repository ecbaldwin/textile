#include "gtest/gtest.h"

namespace {

    class HelloTest : public ::testing::Test {
        protected:
            HelloTest() {}
            virtual ~HelloTest() {}
            virtual void SetUp() {}
            virtual void TearDown() {}
    };

    TEST_F(HelloTest, HelloWorld) {
        EXPECT_EQ(0, 0);
    }

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
