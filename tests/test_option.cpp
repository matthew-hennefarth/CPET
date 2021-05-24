#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <array>

#include <Eigen/Dense>

#include "Exceptions.h"
#include "Option.h"
#include "EFieldVolume.h"

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

TEST(Option, ValidTopoBox) {
  std::string options_file = "Data/valid_options/topo_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  Option option;
  ASSERT_NO_THROW(option = Option{options_file});

  EXPECT_TRUE(option.centerID.isConstant());
  EXPECT_EQ(option.centerID, AtomID::Constants::origin);

  EXPECT_TRUE(option.direction1ID.isConstant());
  EXPECT_EQ(option.direction1ID, AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID.isConstant());
  EXPECT_EQ(option.direction2ID, AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldPoints.empty());
  EXPECT_TRUE(option.calculateEFieldVolumes.empty());

  ASSERT_EQ(option.calculateEFieldTopology.size(), 1);
  auto tr1 = &option.calculateEFieldTopology[0];

  EXPECT_EQ(tr1->numberOfSamples, 10);
  EXPECT_EQ(tr1->volume->type(), "box");
  EXPECT_TRUE(tr1->volume->isInside(Eigen::Vector3d{0.5, 0.5, 0.5}));
  EXPECT_FLOAT_EQ(tr1->volume->maxDim(), 1.0);
}

TEST(Option, ValidTopoBoxAlign) {
  std::string options_file = "Data/valid_options/topo_align_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  Option option;
  ASSERT_NO_THROW(option = Option{options_file});

  EXPECT_FALSE(option.centerID.isConstant());
  EXPECT_EQ(option.centerID, "D:45:CG");

  EXPECT_TRUE(option.direction1ID.isConstant());
  EXPECT_EQ(option.direction1ID, AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID.isConstant());
  EXPECT_EQ(option.direction2ID, AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldPoints.empty());
  EXPECT_TRUE(option.calculateEFieldVolumes.empty());

  ASSERT_EQ(option.calculateEFieldTopology.size(), 1);
  auto tr1 = &option.calculateEFieldTopology[0];

  EXPECT_EQ(tr1->numberOfSamples, 100000);
  EXPECT_EQ(tr1->volume->type(), "box");
  EXPECT_TRUE(tr1->volume->isInside(Eigen::Vector3d{0.5, -21, 0.7}));
  EXPECT_FLOAT_EQ(tr1->volume->maxDim(), 22.0);
}

TEST(Option, TopoNegativeBox) {
  std::string options_file = "Data/invalid_options/topo_neg_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = Option{options_file}, cpet::invalid_option);
}

TEST(Option, Topo4Params) {
  std::string options_file = "Data/invalid_options/topo_4_params";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = Option{options_file}, cpet::invalid_option);
}

TEST(Option, TopoInvalidVolume) {
  std::string options_file = "Data/invalid_options/topo_invalid_volume";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = Option{options_file}, cpet::invalid_option);
}

TEST(Option, Plot3DSimpleValid) {
  std::string options_file = "Data/valid_options/plot3d_simple_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  Option option;
  ASSERT_NO_THROW(option = Option{options_file});
  ASSERT_EQ(option.calculateEFieldVolumes.size(), 1);

  EFieldVolume& efv = option.calculateEFieldVolumes[0];

  EXPECT_TRUE(efv.showPlot());
  EXPECT_FALSE(efv.points().empty());
  std::array<int, 3> expectedDensity = {3, 4, 3};
  EXPECT_EQ(efv.sampleDensity(), expectedDensity);
  EXPECT_EQ(efv.volume().type(), "box");
  EXPECT_FLOAT_EQ(efv.volume().maxDim(), 1.4);

  EXPECT_FALSE(efv.output());

  EXPECT_TRUE(option.calculateEFieldPoints.empty());
  EXPECT_TRUE(option.calculateEFieldTopology.empty());
}

TEST(Option, Plot3DSimpleBoxInvalid_5Params) {
  std::string options_file = "Data/invalid_options/plot3d_simple_box_5_params";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto o = Option{options_file}, cpet::invalid_option);
}

TEST(Option, Plot3dBlockBoxValid) {
  std::string options_file = "Data/valid_options/plot3d_block_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  Option option;
  ASSERT_NO_THROW(option = Option{options_file});
  ASSERT_EQ(option.calculateEFieldVolumes.size(), 2);

  EFieldVolume& efv0 = option.calculateEFieldVolumes[0];
  EFieldVolume& efv1 = option.calculateEFieldVolumes[1];

  EXPECT_TRUE(efv0.showPlot());
  EXPECT_FALSE(efv1.showPlot());

  EXPECT_FALSE(efv0.points().empty());
  EXPECT_FALSE(efv1.points().empty());

  std::array<int, 3> expectedDensity = {5, 4, 5};
  EXPECT_EQ(efv0.sampleDensity(), expectedDensity);
  expectedDensity = {3, 3, 2};
  EXPECT_EQ(efv1.sampleDensity(), expectedDensity);

  EXPECT_EQ(efv0.volume().type(), "box");
  EXPECT_EQ(efv1.volume().type(), "box");

  EXPECT_FLOAT_EQ(efv0.volume().maxDim(), 1.2);
  EXPECT_FLOAT_EQ(efv1.volume().maxDim(), 1.3);

  ASSERT_TRUE(efv0.output());
  EXPECT_FALSE(efv1.output());
  EXPECT_EQ(*efv0.output(), "my3dvolume.dat");
}

TEST(Option, InvalidPlot3dBlockBoxNoDensity) {
  std::string options_file = "Data/invalid_options/plot3d_block_box_nodens";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto o = Option{options_file}, cpet::invalid_option);
}

TEST(Option, InvalidPlot3dBlockNoVolume) {
  std::string options_file = "Data/invalid_options/plot3d_block_novolume";
  ASSERT_TRUE(std::filesystem::exists(options_file));
  EXPECT_THROW(auto o = Option{options_file}, cpet::invalid_option);
}
