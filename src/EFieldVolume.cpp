#include "EFieldVolume.h"

#include <matplot/matplot.h>

void EFieldVolume::plot(
    const std::vector<Eigen::Vector3d>& electricField) const {
  std::vector<double> x;
  x.reserve(points.size());
  std::vector<double> y;
  y.reserve(points.size());
  std::vector<double> z;
  z.reserve(points.size());
  std::vector<double> ex;
  ex.reserve(electricField.size());
  std::vector<double> ey;
  ey.reserve(electricField.size());
  std::vector<double> ez;
  ez.reserve(electricField.size());
  std::vector<double> m;
  m.reserve(electricField.size());

  for (const auto& p : points) {
    x.emplace_back(p[0]);
    y.emplace_back(p[1]);
    z.emplace_back(p[2]);
  }
  for (const auto& e : electricField) {
    ex.emplace_back(e[0]);
    ey.emplace_back(e[1]);
    ez.emplace_back(e[2]);
    m.emplace_back(sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]));
  }
  matplot::quiver3(x, y, z, ex, ey, ez, m, 0.3)->normalize(true).line_width(2);
  matplot::show();
}
