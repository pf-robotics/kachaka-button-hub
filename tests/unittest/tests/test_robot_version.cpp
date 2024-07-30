#include <tuple>

#include <gtest/gtest.h>

#include "robot_version.hpp"

namespace detail {
// Test valid version format parsing
TEST(ParseRobotVersionTest, ValidVersionFormatCase) {
  String version = "3.0.4";
  auto [success, major, minor, patch] = ParseRobotVersion(version);
  EXPECT_TRUE(success);
  EXPECT_EQ(major, 3);
  EXPECT_EQ(minor, 0);
  EXPECT_EQ(patch, 4);
}

// Test invalid version format parsing
TEST(ParseRobotVersionTest, InvalidVersionFormatCase) {
  {
    String version = "3.0";
    auto [success, major, minor, patch] = ParseRobotVersion(version);
    EXPECT_FALSE(success);
  }
}
}  // namespace detail

// Test RobotVersion constructor with valid string
TEST(RobotVersionTest, ConstructorValidString) {
  RobotVersion version("3.10.14");
  EXPECT_TRUE(version.IsReleaseVersion());
  EXPECT_TRUE(version < RobotVersion(4, 0, 0));
  EXPECT_TRUE(version > RobotVersion(2, 9, 9));
  EXPECT_TRUE(version != RobotVersion(3, 0, 4));
  EXPECT_TRUE(version == RobotVersion(3, 10, 14));
}

// Test RobotVersion constructor with invalid string
TEST(RobotVersionTest, ConstructorInvalidString) {
  EXPECT_FALSE(RobotVersion("3.0").IsReleaseVersion());
  EXPECT_FALSE(RobotVersion("3.0.").IsReleaseVersion());
  EXPECT_FALSE(RobotVersion("3..").IsReleaseVersion());
  EXPECT_FALSE(RobotVersion("3..0").IsReleaseVersion());
}

// Test RobotVersion constructor with valid integers
TEST(RobotVersionTest, ConstructorValidIntegers) {
  RobotVersion version(3, 0, 4);
  EXPECT_TRUE(version.IsReleaseVersion());
  EXPECT_EQ(version.operator<(RobotVersion(4, 0, 0)), true);
  EXPECT_EQ(version.operator>(RobotVersion(2, 9, 9)), true);
  EXPECT_EQ(version.operator==(RobotVersion(3, 0, 4)), true);
}

// Test comparison operators
TEST(RobotVersionTest, ComparisonOperators) {
  RobotVersion v1(3, 0, 4);
  RobotVersion v2(3, 1, 0);
  RobotVersion v3(3, 0, 4);

  EXPECT_TRUE(v1 < v2);
  EXPECT_FALSE(v2 < v1);
  EXPECT_TRUE(v2 > v1);
  EXPECT_FALSE(v1 > v2);
  EXPECT_TRUE(v1 == v3);
  EXPECT_FALSE(v1 == v2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
