#include <gtest/gtest.h>

#include "Histogram2D.h"

TEST(Histogram2D, edges) {
  {
    auto edges = cpet::histo::constructEdges(0, 1, 2);
    ASSERT_FALSE(edges.empty());
    ASSERT_EQ(edges.size(), 2);
    EXPECT_EQ(edges.capacity(), 2);

    EXPECT_EQ(edges[0], 0.5);
    EXPECT_EQ(edges[1], 1.0);
  }
  {
    auto edges = cpet::histo::constructEdges(0, 1, 3);
    ASSERT_FALSE(edges.empty());
    ASSERT_EQ(edges.size(), 3);

    EXPECT_NEAR(edges[0], 0.3333333, 0.00001);
    EXPECT_NEAR(edges[1], 0.666666666, 0.0001);
    EXPECT_NEAR(edges[2], 1.0, 0.00000001);
  }
  {
    auto edges = cpet::histo::constructEdges(-1, 1, 2);
    ASSERT_FALSE(edges.empty());
    ASSERT_EQ(edges.size(), 2);
    EXPECT_EQ(edges[0], 0);
    EXPECT_EQ(edges[1], 1);
  }
}

TEST(Histogram2D, edges1Bin) {
  auto edges = cpet::histo::constructEdges(0, 1, 1);
  ASSERT_FALSE(edges.empty());
  ASSERT_EQ(edges.size(), 1);
  EXPECT_EQ(edges.capacity(), 1);

  EXPECT_EQ(edges[0], 1);
}
TEST(Histogram2D, edgesNumericalInaccuracies) {
  auto edges = cpet::histo::constructEdges(0.007, 1.60145, 10);
  ASSERT_EQ(edges.size(), 10);
}

TEST(Histogram2D, edgesOBins) {
  auto edges = cpet::histo::constructEdges(-2, 1.5, 0);
  EXPECT_TRUE(edges.empty());
}

TEST(Histogram2D, edgesNegativeBins) {
  {
    auto edges = cpet::histo::constructEdges(-1, 5, -20);
    EXPECT_TRUE(edges.empty());
  }
  {
    auto edges = cpet::histo::constructEdges(2, 36, -1);
    EXPECT_TRUE(edges.empty());
  }
  {
    auto edges = cpet::histo::constructEdges(-500, -4, -2);
    EXPECT_TRUE(edges.empty());
  }
}

TEST(Histogram2D, maxMoreThanMin) {
  {
    auto edges = cpet::histo::constructEdges(-2, -5, 2);
    EXPECT_TRUE(edges.empty());
  }
  {
    auto edges = cpet::histo::constructEdges(5, 2, 24);
    EXPECT_TRUE(edges.empty());
  }
}

TEST(Histogram2D, histogram2DSimple) {
  {
    std::vector<double> x_data = {0, 1, 1, 2};
    std::vector<double> y_data = {0, 1, 1, 2};

    auto result = cpet::histo::construct2DHistogram(x_data, y_data, {2, 2}, {0, 2}, {0, 2});

    ASSERT_FALSE(result.empty());
    ASSERT_EQ(result.size(), 2);

    EXPECT_EQ(result[0].size(), 2);
    EXPECT_EQ(result[1].size(), 2);

    EXPECT_EQ(result[0][0], 3);
    EXPECT_EQ(result[1][1], 1);
    EXPECT_EQ(result[1][0], 0);
    EXPECT_EQ(result[0][1], 0);
  }
  {
    std::vector<double> x_data = {0, 1, 1, 1.5, 2, 1.5, -1};
    std::vector<double> y_data = {0, 1, 1.5, 0, 2, 2.1, 1.4};

    auto result = cpet::histo::construct2DHistogram(x_data, y_data, {2, 2}, {0, 2}, {0, 2});

    ASSERT_FALSE(result.empty());
    ASSERT_EQ(result.size(), 2);

    EXPECT_EQ(result[0].size(), 2);
    EXPECT_EQ(result[1].size(), 2);

    EXPECT_EQ(result[0][0], 2);
    EXPECT_EQ(result[1][1], 1);
    EXPECT_EQ(result[1][0], 1);
    EXPECT_EQ(result[0][1], 1);
  }
}
