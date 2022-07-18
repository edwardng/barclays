#include <sstream>
#include <stdexcept>

#include "RandomDistributionGeneratorFactory.h"

namespace basket::pricer {
std::unique_ptr<IRandomDistributionGenerator>
RandomDistributionGeneratorFactory::create(const std::vector<std::string> &params) {
  std::string random_model = params[0];

  if (params.size() < 2)
	throw std::invalid_argument("Missing distribution argument");

  std::istringstream iss(params[1]);
  double args1{0};
  iss >> args1;

  if (random_model == "poisson_distribution") {
	return std::unique_ptr<DistributionGenerator<std::poisson_distribution<>>>
		(new DistributionGenerator<std::poisson_distribution<>>(args1));
  }

  if (params.size() != 3)
	throw std::invalid_argument("Unexpected distribution argument");

  iss.clear();
  iss.str(params[2]);
  double args2{0};
  iss >> args2;

  if (random_model == "uniform_real_distribution") {
	return std::unique_ptr<DistributionGenerator<std::uniform_real_distribution<>>>
		(new DistributionGenerator<std::uniform_real_distribution<>>(args1, args2));
  }
  if (random_model == "uniform_int_distribution") {
	return std::unique_ptr<DistributionGenerator<std::uniform_int_distribution<>>>
		(new DistributionGenerator<std::uniform_int_distribution<>>(args1, args2));
  }
  if (random_model == "normal_distribution") {
	return std::unique_ptr<DistributionGenerator<std::normal_distribution<>>>
		(new DistributionGenerator<std::normal_distribution<>>(args1, args2));
  }

  throw std::invalid_argument("Distribution not support");
}
}