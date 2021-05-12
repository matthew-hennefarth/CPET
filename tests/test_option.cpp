#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <iostream>

#include <Eigen/Core>

#include "Exceptions.h"
#include "Option.h"

TEST(Option, SimpleField) {
  ASSERT_TRUE(std::filesystem::exists("Data/valid_options/simple_field"));

  Option option;
  ASSERT_NO_THROW(option = Option{"Data/valid_options/simple_field"});

  EXPECT_TRUE(option.centerID.isConstant());
  EXPECT_EQ(option.centerID, AtomID::Constants::origin);

  EXPECT_TRUE(option.direction1ID.isConstant());
  EXPECT_EQ(option.direction1ID, AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID.isConstant());
  EXPECT_EQ(option.direction2ID, AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldTopology.empty());
  EXPECT_TRUE(option.calculateEFieldVolumes.empty());

  ASSERT_EQ(option.calculateEFieldPoints.size(), 1);
  EXPECT_EQ(option.calculateEFieldPoints[0], "0:0:0");
}

TEST(Option, AlignZero) {
  std::string options_file = "Data/invalid_options/align_zero";
  ASSERT_TRUE(std::filesystem::exists(options_file));
  EXPECT_THROW(Option{options_file}, cpet::invalid_option);
}

TEST(Option, AlignSingle) {
  ASSERT_TRUE(std::filesystem::exists("Data/valid_options/align_single"));

  Option option;
  ASSERT_NO_THROW(option = Option{"Data/valid_options/align_single"});

  EXPECT_FALSE(option.centerID.isConstant());
  EXPECT_TRUE(option.centerID.isVector());
  Eigen::Vector3d center{1, 0, 0};
  ASSERT_TRUE(option.centerID.position());
  EXPECT_NEAR((*option.centerID.position() - center).norm(), 0.0, 0.0001);

  EXPECT_TRUE(option.direction1ID.isConstant());
  EXPECT_EQ(option.direction1ID, AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID.isConstant());
  EXPECT_EQ(option.direction2ID, AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldTopology.empty());
  EXPECT_TRUE(option.calculateEFieldVolumes.empty());

  ASSERT_EQ(option.calculateEFieldPoints.size(), 1);
  EXPECT_EQ(option.calculateEFieldPoints[0], "5:4:2");
}

TEST(Option, AlignDouble) {
  std::string options_file = "Data/invalid_options/align_double";
  ASSERT_TRUE(std::filesystem::exists(options_file));
 
  EXPECT_THROW(Option{options_file}, cpet::invalid_option);
}

TEST(Option, AlignTriple) {
  std::string options_file = "Data/valid_options/align_triple";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  Option option;
  ASSERT_NO_THROW(option = Option{options_file});

  EXPECT_FALSE(option.centerID.isConstant());
  EXPECT_TRUE(option.centerID.isVector());
  Eigen::Vector3d center{1, 3, 1};
  ASSERT_TRUE(option.centerID.position());
  EXPECT_NEAR((*option.centerID.position() - center).norm(), 0.0, 0.0001);

  EXPECT_FALSE(option.direction1ID.isConstant());
  EXPECT_FALSE(option.direction1ID.isVector());
  EXPECT_EQ(option.direction1ID, "D:54:CG");

  EXPECT_FALSE(option.direction2ID.isConstant());
  EXPECT_FALSE(option.direction2ID.isVector());
  EXPECT_EQ(option.direction2ID, "D:56:SG");

  EXPECT_TRUE(option.calculateEFieldTopology.empty());
  EXPECT_TRUE(option.calculateEFieldVolumes.empty());

  ASSERT_EQ(option.calculateEFieldPoints.size(), 2);
  EXPECT_EQ(option.calculateEFieldPoints[0], "54:55:34");
  EXPECT_EQ(option.calculateEFieldPoints[1], "D:5:C100");
}
