#include <gtest/gtest.h>

#include <vector>
#include <iostream>

#include <Eigen/Dense>

#include "Option.h"
#include "System.h"
#include "PointCharge.h"

TEST(System, SimpleField) {
  Option option;
  option.calculateEFieldPoints.push_back(AtomID("1:1:1"));

  std::vector<PointCharge> pc;
  pc.emplace_back(Eigen::Vector3d{0, 0, 0}, 1, AtomID{"A:1:NH"});
  {
    System sys{pc, option};
    EXPECT_FLOAT_EQ(sys.center().norm(), 0.0);

    Eigen::Matrix3d identity = Eigen::Matrix3d::Identity();

    EXPECT_EQ(sys.basisMatrix(), identity);

    Eigen::Vector3d field =
        sys.electricFieldAt(*option.calculateEFieldPoints[0].position());

    Eigen::Vector3d expected_result{2.77121, 2.77121, 2.77121};

    EXPECT_NEAR((field - expected_result).norm(), 0, 0.00001);
  }

  option.centerID = AtomID("1:1:1");

  {
    System sys{pc, option};
    sys.transformToUserSpace();
    Eigen::Vector3d expected_center{1, 1, 1};
    EXPECT_NEAR((sys.center() - expected_center).norm(), 0, 0.00001);

    Eigen::Matrix3d identity = Eigen::Matrix3d::Identity();
    EXPECT_EQ(sys.basisMatrix(), identity);

    Eigen::Vector3d loc =
        sys.transformToUserSpace(*option.calculateEFieldPoints[0].position());

    Eigen::Vector3d field = sys.electricFieldAt(loc);
    Eigen::Vector3d expected_result{2.77121, 2.77121, 2.77121};
    EXPECT_NEAR((field - expected_result).norm(), 0, 0.00001);
  }
}