#include <gtest/gtest.h>

#include <Eigen/Core>
#include <exception>
#include <string>
#include <vector>

#include "AtomID.h"
#include "Exceptions.h"
#include "PointCharge.h"
#include "Frame.h"

TEST(AtomID, GenerateFromPDB) {
  const std::vector<std::string> pdbLines = {
      "HETATM 5719 O110 PRE D   2     113.861  94.989 107.751 -0.846  1.520",
      "HETATM 5720 C111 PRE D   2     112.558  98.013 111.038 -0.447  1.700",
      "HETATM 5721 C112 PRE D   2     111.435  97.635 110.419  0.318  1.700",
      "HETATM 5722 O113 PRE D   2     110.611  96.683 110.992 -0.826  1.520",
      "HETATM 5723 C114 PRE D   2     110.934  98.331 109.158  0.765  1.700",
      "HETATM 5724 O115 PRE D   2     111.734  98.657 108.273 -0.830  1.520",
      "HETATM 5725 O116 PRE D   2     109.671  98.566 109.167 -0.834  1.520",
      "HETATM 5726 H117 PRE D   2     112.589  95.154 109.851  0.280  1.200",
      "HETATM 5727 H118 PRE D   2     110.416  94.662 111.078  0.278  1.200",
      "HETATM 5728 H119 PRE D   2     108.459  96.170 109.927  0.265  1.200",
      "HETATM 5729 H120 PRE D   2     108.093  95.502 107.605  0.221  1.200",
      "HETATM 5730 H121 PRE D   2     110.220  95.496 106.277  0.268  1.200"};

  std::vector<std::string> ids = {
      "D:2:O110", "D:2:C111", "D:2:C112", "D:2:O113", "D:2:C114", "D:2:O115",
      "D:2:O116", "D:2:H117", "D:2:H118", "D:2:H119", "D:2:H120", "D:2:H121"};
  for (size_t i = 0; i < pdbLines.size(); i++) {
    auto a =
        cpet::AtomID::generateID(pdbLines[i], cpet::constants::FileType::pdb);
    ASSERT_EQ(a.ID(), ids[i]);
    ASSERT_FALSE(a.isConstant())
        << "id " << a.ID() << " should not be a constant";
    ASSERT_FALSE(a.position())
        << "id " << a.ID() << " should not have a position";
    ASSERT_FALSE(a.isVector()) << "id " << a.ID() << " should not be a vector";
  }

  EXPECT_NO_THROW(
      auto a = cpet::AtomID::generateID("HETATM 5719 O110 PRE D   2 113.861",
                                        cpet::constants::FileType::pdb));
  EXPECT_THROW(auto a = cpet::AtomID::generateID(
                   "HETATM 5719 O110 PRE D", cpet::constants::FileType::pdb),
               cpet::value_error);
}

TEST(AtomID, ConstructFromPQR) {
  const std::vector<std::string> pqrlines = {
      "HETATM 4519 HMB3 HEM A 1300       9.944  13.041  -3.295  0.058  1.200"
      "HETATM 4520 HMC1 HEM A 1300       2.780  17.583  -0.593  0.063  1.200"
      "HETATM 4521 HMC2 HEM A 1300       3.872  16.247  -0.158  0.063  1.200"
      "HETATM 4522 HMC3 HEM A 1300       3.314  17.385   1.093  0.063  1.200"
      "HETATM 4523 HMD1 HEM A 1300       6.118  24.087  -0.950  0.070  1.200"
      "HETATM 4524 HMD2 HEM A 1300       7.469  25.207  -1.249  0.065  1.200"
      "HETATM 4525 HMD3 HEM A 1300       7.344  24.360   0.312  0.063  1.200"
      "HETATM 4526 HMA1 HEM A 1300      12.245  17.272  -6.535  0.074  1.200"
      "HETATM 4527 HMA2 HEM A 1300      13.575  18.410  -6.215  0.078  1.200"
      "HETATM 4528 HMA3 HEM A 1300      13.337  16.999  -5.157  0.065  1.200"};
  std::vector<std::string> ids = {"A:1300:HMB3", "A:1300:HMC1", "A:1300:HMC2",
                                  "A:1300:HMC3", "A:1300:HMD1", "A:1300:HMD2",
                                  "A:1300:HMD3", "A:1300:HMA1", "A:1300:HMA2",
                                  "A:1300:HMA3"};

  for (size_t i = 0; i < pqrlines.size(); i++) {
    auto a =
        cpet::AtomID::generateID(pqrlines[i], cpet::constants::FileType::pqr);
    ASSERT_EQ(a.ID(), ids[i]);
    ASSERT_FALSE(a.isConstant())
        << "id " << a.ID() << " should not be a constant";
    ASSERT_FALSE(a.position())
        << "id " << a.ID() << " should not have a position";
    ASSERT_FALSE(a.isVector()) << "id " << a.ID() << " should not be a vector";
  }

  EXPECT_NO_THROW(auto a = cpet::AtomID::generateID(
                      "HETATM 4528 HMA3 HEM A 1300      13.337",
                      cpet::constants::FileType::pdb));
  EXPECT_THROW(auto a = cpet::AtomID::generateID(
                   "HETATM 4528 HMA3 HEM A", cpet::constants::FileType::pdb),
               cpet::value_error);
}

TEST(AtomID, ConstructWithString) {
  EXPECT_NO_THROW(cpet::AtomID("D:115:C101"));
  EXPECT_THROW(cpet::AtomID("A:152"), cpet::value_error);
  EXPECT_THROW(cpet::AtomID("A:hg2:f4"), cpet::value_error);
  EXPECT_THROW(cpet::AtomID("A 115 C203"), cpet::value_error);
  EXPECT_THROW(
      cpet::AtomID("HETATM 5725 O116 PRE D   2     109.671  98.566 109.167 "
                   "-0.834  1.520"),
      cpet::value_error);
}

TEST(AtomID, AssignWithString) {
  cpet::AtomID a("D:115:C101");

  ASSERT_EQ(a.ID(), "D:115:C101");
  a = "D:2:H117";
  ASSERT_EQ(a.ID(), "D:2:H117");

  EXPECT_NO_THROW(a = "D:2:H117");
  EXPECT_NO_THROW(a = "GF:254:C107");
  EXPECT_NO_THROW(a = "GF:56 :a");
  EXPECT_THROW(a = "Gaa:asdf:fff", cpet::value_error);
  EXPECT_THROW(a = "HETATM 5725 O116 PRE D   2     109.671  98.566 109.167",
               cpet::value_error);
}

TEST(AtomID, AssignStringToVector) {
  cpet::AtomID a("106:102:108");
  Eigen::Vector3d a_vector = {106, 102, 108};

  ASSERT_TRUE(a.position());
  ASSERT_TRUE(a.isVector());
  EXPECT_FALSE(a.isConstant());
  EXPECT_EQ(a.position(), a_vector);

  ASSERT_NO_THROW(a = "GF:254:C107");
  EXPECT_FALSE(a.isConstant());
  EXPECT_FALSE(a.position());
  EXPECT_FALSE(a.isVector());
}

TEST(AtomID, ConstructVector) {
  ASSERT_NO_THROW(cpet::AtomID("105:1:200"));

  cpet::AtomID a("106:102:108");
  Eigen::Vector3d a_vector = {106, 102, 108};
  EXPECT_TRUE(a.isVector());
  EXPECT_FALSE(a.isConstant());
  ASSERT_TRUE(a.position());
  EXPECT_EQ(a.position(), a_vector);

  cpet::AtomID b("-45.2:35.1231:452.200");
  Eigen::Vector3d b_vector = {-45.2, 35.1231, 452.200};
  EXPECT_TRUE(b.isVector());
  EXPECT_FALSE(b.isConstant());
  ASSERT_TRUE(b.position());
  EXPECT_EQ(b.position(), b_vector);
}

TEST(AtomID, AssignVectorToString) {
  cpet::AtomID a("D:115:C101");
  ASSERT_FALSE(a.position());
  EXPECT_FALSE(a.isVector());
  EXPECT_FALSE(a.isConstant());
  a = "106:102:108";
  EXPECT_TRUE(a.isVector());
  ASSERT_TRUE(a.position());
  Eigen::Vector3d a_vector = {106, 102, 108};
  EXPECT_EQ(a.position(), a_vector);
  EXPECT_FALSE(a.isConstant());
}

TEST(AtomID, OriginConstants) {
  cpet::AtomID a(cpet::AtomID::Constants::origin);
  EXPECT_TRUE(a.isConstant());
  EXPECT_TRUE(a.isVector());
  ASSERT_TRUE(a.position());
  EXPECT_EQ(a.position(), Eigen::Vector3d({0, 0, 0}));
}

TEST(AtomID, e1Constants) {
  cpet::AtomID a(cpet::AtomID::Constants::e1);
  EXPECT_TRUE(a.isConstant());
  EXPECT_TRUE(a.isVector());
  ASSERT_TRUE(a.position());
  EXPECT_EQ(a.position(), Eigen::Vector3d({1, 0, 0}));
}

TEST(AtomID, e2Constants) {
  cpet::AtomID a(cpet::AtomID::Constants::e2);
  EXPECT_TRUE(a.isConstant());
  EXPECT_TRUE(a.isVector());
  ASSERT_TRUE(a.position());
  EXPECT_EQ(a.position(), Eigen::Vector3d({0, 1, 0}));
}

TEST(AtomID, AssignStringToConstant) {
  cpet::AtomID a(cpet::AtomID::Constants::e2);
  ASSERT_TRUE(a.isConstant());
  a = "D:5:C100";
  EXPECT_FALSE(a.isConstant());
  EXPECT_FALSE(a.isVector());
  EXPECT_FALSE(a.position());
}

TEST(AtomID, AssignVectorToConstant) {
  cpet::AtomID a(cpet::AtomID::Constants::e2);
  ASSERT_TRUE(a.isConstant());
  ASSERT_EQ(a.position(), Eigen::Vector3d({0, 1, 0}));

  a = "105.3:-303.00:299";
  EXPECT_FALSE(a.isConstant());
  EXPECT_TRUE(a.isVector());
  EXPECT_TRUE(a.position());
  EXPECT_EQ(a.position(), Eigen::Vector3d({105.3, -303.00, 299}));
}

TEST(PointCharge, find) {
  cpet::PointCharge pc1{Eigen::Vector3d{3.0, 2.0, 2.5}, 1.0,
                        cpet::AtomID{"A:4:CD"}};
  cpet::PointCharge pc2{Eigen::Vector3d{3.5, 1.0, 3.5}, 1.0,
                        cpet::AtomID{"C:35:SG\'"}};
  cpet::PointCharge pc3{Eigen::Vector3d{-1.0, 0.92, 6.5}, 1.0,
                        cpet::AtomID{"A:45:C100"}};
  cpet::PointCharge pc4{Eigen::Vector3d{4.23, 8.0, -2.5}, 1.0,
                        cpet::AtomID{"B:200:HB"}};

  cpet::Frame system({pc1, pc2, pc3, pc4});

  EXPECT_EQ(*system.find(cpet::AtomID{"C:35:SG\'"}), pc2);
  EXPECT_EQ(*system.find(cpet::AtomID{"A:4:CD"}), pc1);
  EXPECT_EQ(*system.find(cpet::AtomID{"B:200:HB"}), pc4);

  EXPECT_THROW((void)system.find(cpet::AtomID("C:35:SG")),
               cpet::value_not_found);
  EXPECT_THROW((void)system.find(cpet::AtomID("D:54:C102")),
               cpet::value_not_found);
}

// test pointcharges (some of the basic functionalities...)
