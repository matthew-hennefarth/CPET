#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <array>

#include <Eigen/Dense>

#include "Exceptions.h"
#include "Option.h"
#include "EFieldVolume.h"
#include "FieldLocations.h"

TEST(Option, SimpleField) {
  ASSERT_TRUE(std::filesystem::exists("Data/valid_options/simple_field"));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{"Data/valid_options/simple_field"});

  EXPECT_TRUE(option.centerID().isConstant());
  EXPECT_EQ(option.centerID(), cpet::AtomID::Constants::origin);

  EXPECT_TRUE(option.direction1ID().isConstant());
  EXPECT_EQ(option.direction1ID(), cpet::AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID().isConstant());
  EXPECT_EQ(option.direction2ID(), cpet::AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldTopology().empty());
  EXPECT_TRUE(option.calculateEFieldVolumes().empty());

  ASSERT_EQ(option.calculateFieldLocations().size(), 1);
  auto fl = option.calculateFieldLocations()[0];
  ASSERT_EQ(fl.locations().size(), 1);
  EXPECT_EQ(fl.locations()[0], "0:0:0");

  EXPECT_EQ(option.coordinatesStartIndex(), 0);
  EXPECT_EQ(option.coordinatesStepSize(), 1);
}

TEST(Option, AlignZero) {
  std::string options_file = "Data/invalid_options/align_zero";
  ASSERT_TRUE(std::filesystem::exists(options_file));
  EXPECT_THROW(cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, AlignSingle) {
  ASSERT_TRUE(std::filesystem::exists("Data/valid_options/align_single"));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{"Data/valid_options/align_single"});

  EXPECT_FALSE(option.centerID().isConstant());
  EXPECT_TRUE(option.centerID().isVector());
  Eigen::Vector3d center{1, 0, 0};
  ASSERT_TRUE(option.centerID().position());
  EXPECT_NEAR((*option.centerID().position() - center).norm(), 0.0, 0.0001);

  EXPECT_TRUE(option.direction1ID().isConstant());
  EXPECT_EQ(option.direction1ID(), cpet::AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID().isConstant());
  EXPECT_EQ(option.direction2ID(), cpet::AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateEFieldTopology().empty());
  EXPECT_TRUE(option.calculateEFieldVolumes().empty());

  ASSERT_EQ(option.calculateFieldLocations().size(), 1);
  auto fl = option.calculateFieldLocations()[0];
  ASSERT_EQ(fl.locations().size(), 1);
  EXPECT_EQ(fl.locations()[0], "5:4:2");

  EXPECT_EQ(option.coordinatesStartIndex(), 0);
  EXPECT_EQ(option.coordinatesStepSize(), 1);
}

TEST(Option, AlignDouble) {
  std::string options_file = "Data/invalid_options/align_double";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, AlignTriple) {
  std::string options_file = "Data/valid_options/align_triple";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});

  EXPECT_FALSE(option.centerID().isConstant());
  EXPECT_TRUE(option.centerID().isVector());
  Eigen::Vector3d center{1, 3, 1};
  ASSERT_TRUE(option.centerID().position());
  EXPECT_NEAR((*option.centerID().position() - center).norm(), 0.0, 0.0001);

  EXPECT_FALSE(option.direction1ID().isConstant());
  EXPECT_FALSE(option.direction1ID().isVector());
  EXPECT_EQ(option.direction1ID(), "D:54:CG");

  EXPECT_FALSE(option.direction2ID().isConstant());
  EXPECT_FALSE(option.direction2ID().isVector());
  EXPECT_EQ(option.direction2ID(), "D:56:SG");

  EXPECT_TRUE(option.calculateEFieldTopology().empty());
  EXPECT_TRUE(option.calculateEFieldVolumes().empty());

  ASSERT_EQ(option.calculateFieldLocations().size(), 1);
  auto fl = option.calculateFieldLocations()[0];
  ASSERT_EQ(fl.locations().size(), 2);
  EXPECT_EQ(fl.locations()[0], "54:55:34");
  EXPECT_EQ(fl.locations()[1], "D:5:C100");
}

TEST(Option, ValidTopoBox) {
  std::string options_file = "Data/valid_options/topo_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});

  EXPECT_TRUE(option.centerID().isConstant());
  EXPECT_EQ(option.centerID(), cpet::AtomID::Constants::origin);

  EXPECT_TRUE(option.direction1ID().isConstant());
  EXPECT_EQ(option.direction1ID(), cpet::AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID().isConstant());
  EXPECT_EQ(option.direction2ID(), cpet::AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateFieldLocations().empty());
  EXPECT_TRUE(option.calculateEFieldVolumes().empty());

  ASSERT_EQ(option.calculateEFieldTopology().size(), 1);
  const auto* tr1 = &option.calculateEFieldTopology()[0];

  EXPECT_EQ(tr1->numberOfSamples(), 10);
  EXPECT_EQ(tr1->volume().type(), "box");
  EXPECT_TRUE(tr1->volume().isInside(Eigen::Vector3d{0.5, 0.5, 0.5}));
  EXPECT_FLOAT_EQ(tr1->volume().maxDim(), 1.0);

  EXPECT_EQ(option.coordinatesStartIndex(), 0);
  EXPECT_EQ(option.coordinatesStepSize(), 1);
}

TEST(Option, ValidTopoBoxAlign) {
  std::string options_file = "Data/valid_options/topo_align_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});

  EXPECT_FALSE(option.centerID().isConstant());
  EXPECT_EQ(option.centerID(), "D:45:CG");

  EXPECT_TRUE(option.direction1ID().isConstant());
  EXPECT_EQ(option.direction1ID(), cpet::AtomID::Constants::e1);

  EXPECT_TRUE(option.direction2ID().isConstant());
  EXPECT_EQ(option.direction2ID(), cpet::AtomID::Constants::e2);

  EXPECT_TRUE(option.calculateFieldLocations().empty());
  EXPECT_TRUE(option.calculateEFieldVolumes().empty());

  ASSERT_EQ(option.calculateEFieldTopology().size(), 1);
  const auto* tr1 = &option.calculateEFieldTopology()[0];

  EXPECT_EQ(tr1->numberOfSamples(), 100000);
  EXPECT_EQ(tr1->volume().type(), "box");
  EXPECT_TRUE(tr1->volume().isInside(Eigen::Vector3d{0.5, -21, 0.7}));
  EXPECT_FLOAT_EQ(tr1->volume().maxDim(), 22.0);

  EXPECT_EQ(option.coordinatesStartIndex(), 0);
  EXPECT_EQ(option.coordinatesStepSize(), 1);
}

TEST(Option, TopoNegativeBox) {
  std::string options_file = "Data/invalid_options/topo_neg_box";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, Topo4Params) {
  std::string options_file = "Data/invalid_options/topo_4_params";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, TopoInvalidVolume) {
  std::string options_file = "Data/invalid_options/topo_invalid_volume";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, Plot3DSimpleValid) {
  std::string options_file = "Data/valid_options/plot3d_simple_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  ASSERT_EQ(option.calculateEFieldVolumes().size(), 1);

  const cpet::EFieldVolume& efv = option.calculateEFieldVolumes()[0];

  EXPECT_TRUE(efv.showPlot());
  EXPECT_FALSE(efv.points().empty());
  std::array<int, 3> expectedDensity = {3, 4, 3};
  EXPECT_EQ(efv.sampleDensity(), expectedDensity);
  EXPECT_EQ(efv.volume().type(), "box");
  EXPECT_FLOAT_EQ(efv.volume().maxDim(), 1.4);

  EXPECT_FALSE(efv.output());

  EXPECT_TRUE(option.calculateFieldLocations().empty());
  EXPECT_TRUE(option.calculateEFieldTopology().empty());
}

TEST(Option, Plot3DSimpleBoxInvalid_5Params) {
  std::string options_file = "Data/invalid_options/plot3d_simple_box_5_params";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  EXPECT_THROW(auto o = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, Plot3dBlockBoxValid) {
  std::string options_file = "Data/valid_options/plot3d_block_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  ASSERT_EQ(option.calculateEFieldVolumes().size(), 2);

  const cpet::EFieldVolume& efv0 = option.calculateEFieldVolumes()[0];
  const cpet::EFieldVolume& efv1 = option.calculateEFieldVolumes()[1];

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

  EXPECT_THROW(auto o = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, InvalidPlot3dBlockNoVolume) {
  std::string options_file = "Data/invalid_options/plot3d_block_novolume";
  ASSERT_TRUE(std::filesystem::exists(options_file));
  EXPECT_THROW(auto o = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, FieldBlockValid) {
  std::string options_file = "Data/valid_options/field_block_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});

  ASSERT_EQ(option.calculateFieldLocations().size(), 1);
  auto fl = option.calculateFieldLocations()[0];

  ASSERT_EQ(fl.locations().size(), 2);

  cpet::PlotStyles plotstyle = fl.plotStyle();
  EXPECT_TRUE(fl.showPlots());
  EXPECT_TRUE((plotstyle & cpet::PlotStyles::x) == cpet::PlotStyles::x);
  EXPECT_TRUE((plotstyle & cpet::PlotStyles::m) == cpet::PlotStyles::m);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::y) == cpet::PlotStyles::y);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::z) == cpet::PlotStyles::z);
  EXPECT_TRUE(fl.output());

  EXPECT_EQ(*fl.output(), "fields_ab");
}

TEST(Option, Field2BlockValid) {
  std::string options_file = "Data/valid_options/field_2block_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});

  ASSERT_EQ(option.calculateFieldLocations().size(), 2);
  auto fl1 = option.calculateFieldLocations()[0];

  ASSERT_EQ(fl1.locations().size(), 1);
  EXPECT_EQ(fl1.locations()[0], "1:2:1");

  cpet::PlotStyles plotstyle = fl1.plotStyle();
  EXPECT_TRUE(fl1.showPlots());
  EXPECT_TRUE((plotstyle & cpet::PlotStyles::x) == cpet::PlotStyles::x);
  EXPECT_TRUE((plotstyle & cpet::PlotStyles::y) == cpet::PlotStyles::y);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::m) == cpet::PlotStyles::m);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::z) == cpet::PlotStyles::z);
  EXPECT_FALSE(fl1.output());

  auto fl2 = option.calculateFieldLocations()[1];
  ASSERT_EQ(fl2.locations().size(), 2);
  EXPECT_EQ(fl2.locations()[0], "C:126:SG");
  EXPECT_EQ(fl2.locations()[1], "45:64:3");

  plotstyle = fl2.plotStyle();
  EXPECT_FALSE(fl2.showPlots());
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::x) == cpet::PlotStyles::x);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::y) == cpet::PlotStyles::y);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::m) == cpet::PlotStyles::m);
  EXPECT_FALSE((plotstyle & cpet::PlotStyles::z) == cpet::PlotStyles::z);

  ASSERT_TRUE(fl2.output());
  EXPECT_EQ(*fl2.output(), "2locations.data");
}

TEST(Option, FieldBlockNoLocations) {
  std::string options_file = "Data/valid_options/field_block_nolocations";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  EXPECT_EQ(option.calculateFieldLocations().size(), 1);
  EXPECT_TRUE(option.calculateFieldLocations()[0].locations().empty());
}

TEST(Option, FieldBlockInvalidPlot) {
  std::string options_file = "Data/invalid_options/field_block_invalidplot";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_THROW(option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, TopologyBlockValid) {
  std::string options_file = "Data/valid_options/topology_block_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  ASSERT_FALSE(option.calculateEFieldTopology().empty());

  const auto& tr = option.calculateEFieldTopology()[0];
  EXPECT_EQ(tr.stepSize(), 0.1);
  EXPECT_EQ(tr.numberOfSamples(), 150);
  EXPECT_EQ(tr.sampleOutput(), "topo_prefix");

  const auto& vol = tr.volume();
  EXPECT_EQ(vol.type(), "box");
  EXPECT_TRUE(vol.isInside(Eigen::Vector3d{0.5, 0.5, 0.5}));
  EXPECT_FLOAT_EQ(vol.maxDim(), 2.0);
}

TEST(Option, TopologyBlockNoVolume) {
  std::string options_file = "Data/invalid_options/topo_block_novolume";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_THROW(option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, TopologyBlockNoSamples) {
  std::string options_file = "Data/invalid_options/topo_block_nosamples";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_THROW(option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, StartStepValid) {
  std::string options_file = "Data/valid_options/start_step_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  ASSERT_FALSE(option.calculateFieldLocations().empty());

  EXPECT_EQ(option.coordinatesStartIndex(), 50);
  EXPECT_EQ(option.coordinatesStepSize(), 4);
}

TEST(Option, StartNonNumericInvalid) {
  std::string options_file = "Data/invalid_options/start_nonnumeric";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_THROW(option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, Skip0Invalid) {
  std::string options_file = "Data/invalid_options/skip_0";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_THROW(option = cpet::Option{options_file}, cpet::invalid_option);
}

TEST(Option, StartStepEmptyValid) {
  std::string options_file = "Data/valid_options/start_step_empty_valid";
  ASSERT_TRUE(std::filesystem::exists(options_file));

  cpet::Option option;
  ASSERT_NO_THROW(option = cpet::Option{options_file});
  ASSERT_FALSE(option.calculateFieldLocations().empty());

  EXPECT_EQ(option.coordinatesStartIndex(), 0);
  EXPECT_EQ(option.coordinatesStepSize(), 1);
}