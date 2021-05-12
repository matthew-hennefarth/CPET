#include <gtest/gtest.h>

#include <Eigen/Core>
#include <array>
#include <vector>

#include "Box.h"
#include "Exceptions.h"

TEST(Box, BasicProperties) {
  Box b({1.3, 2.4, 2});

  ASSERT_EQ(b.type(), "box") << "Incorrectly describes the volume type";
  ASSERT_EQ(b.description(), "Box: 1.300000 2.400000 2.000000")
      << "Incorrect description and or formatting";

  EXPECT_DOUBLE_EQ(b.maxDim(), 2.4) << "Expected maximum dimension " << 2.4;
  EXPECT_NEAR(b.diagonal(), 6.76756973, 0.0000001)
      << "Diagonal off by expected " << 6.76756973 << " by more than tolerance "
      << 0.0000001;

  Eigen::Vector3d point{0, 0, 0};
  EXPECT_TRUE(b.isInside(point)) << "Origin is not within the box";

  point = {1.2, -2, 1};
  EXPECT_TRUE(b.isInside(point))
      << "Point " << point.transpose() << " not within " << b.description();

  point = {-1.3, 2.4, 2.1};
  EXPECT_FALSE(b.isInside(point))
      << "Point " << point.transpose() << " within " << b.description();

  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(b.isInside(b.randomPoint()));
  }

  constexpr double STEP_SIZE = 0.001;
  const double max_distance = b.diagonal() / STEP_SIZE;
  for (int i = 0; i < 10; i++) {
    EXPECT_TRUE(b.randomDistance(STEP_SIZE) <= max_distance);
  }
}
TEST(Box, Partition) {
  std::array<double, 3> sides = {2, 3, 5};
  std::array<int, 3> density = {10, 10, 10};
  Box b(sides);

  std::vector<Eigen::Vector3d> expected_partition;
  double x = -1 * sides[0];
  while (x < sides[0]) {
    double y = -1 * sides[1];
    while (y < sides[1]) {
      double z = -1 * sides[2];
      while (z < sides[2]) {
        expected_partition.emplace_back(x, y, z);
        z += static_cast<double>(static_cast<float>(sides[2] / density[2]));
      }
      y += (sides[1] / density[1]);
    }
    x += (sides[0] / density[0]);
  }

  auto result = b.partition(density);
  ASSERT_EQ(result.size(), expected_partition.size());
  for (size_t i = 0; i < result.size(); i++) {
    double difference = (result.at(i) - expected_partition.at(i)).norm();
    EXPECT_NEAR(difference, 0.0, 0.000001);
  }
}

TEST(Box, InvalidParameters) {
  EXPECT_THROW(Box({-1.5, 2, 3}), cpet::value_error);
  EXPECT_THROW(Box({1.5, -2, 3}), cpet::value_error);
  EXPECT_THROW(Box({1.5, 2, -3}), cpet::value_error);
  EXPECT_THROW(Box({-1.5, 2, -3}), cpet::value_error);
}
