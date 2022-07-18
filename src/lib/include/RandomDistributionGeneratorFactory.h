#pragma once

#include <random>
#include <memory>
#include <vector>

namespace basket::pricer {
class IRandomDistributionGenerator {
 public:
  virtual double getNextValue() = 0;
};

template<typename D>
class DistributionGenerator : public IRandomDistributionGenerator {
 public:
  template<typename... Args>
  DistributionGenerator(Args &&... args)
	  : rd(), generator(rd()), distribution(std::forward<Args>(args)...) {
  }

  double getNextValue() {
	return distribution(generator);
  }

 protected:
  std::random_device rd;
  std::mt19937 generator;
  D distribution;
};

struct RandomDistributionGeneratorFactory {
  static std::unique_ptr<IRandomDistributionGenerator> create(const std::vector<std::string> &params);
};
}