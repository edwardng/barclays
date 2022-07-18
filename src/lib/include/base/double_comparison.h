#pragma once

#include <cmath>
#include <limits>

namespace basket::pricer {

inline bool double_equal(double f1, double f2) {
  return (std::fabs(f1 - f2) <= std::numeric_limits<double>::epsilon() * std::fmax(std::fabs(f1), std::fabs(f2)));
}

}