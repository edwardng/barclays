#pragma once

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "base/types.h"

#include "RandomDistributionGeneratorFactory.h"

namespace basket::pricer {
enum class Side : uint16_t {
  BID,
  ASK
};

struct InstrumentSimulationModel {
 public:

  InstrumentSimulationModel() = delete;

  InstrumentSimulationModel(std::unique_ptr<IRandomDistributionGenerator> next_event_time_rg,
							std::unique_ptr<IRandomDistributionGenerator> initial_price_rg,
							std::unique_ptr<IRandomDistributionGenerator> direction_rg,
							std::unique_ptr<IRandomDistributionGenerator> tick_move_rg,
							std::unique_ptr<IRandomDistributionGenerator> side_rg,
							const int &max_tick_diff)
	  : next_event_time_rg_(std::move(next_event_time_rg)), initial_price_rg_(std::move(initial_price_rg)),
		direction_rg_(std::move(direction_rg)), tick_move_rg_(std::move(tick_move_rg)),
		side_rg_(std::move(side_rg)), max_tick_diff_(max_tick_diff) {
  }

  InstrumentSimulationModel(const InstrumentSimulationModel &) = delete;

  InstrumentSimulationModel &operator=(const InstrumentSimulationModel &) = delete;

  InstrumentSimulationModel(InstrumentSimulationModel &&) noexcept = default;

  InstrumentSimulationModel &operator=(InstrumentSimulationModel &&) noexcept = default;

  ~InstrumentSimulationModel() = default;

  inline std::uint64_t getNextEventTime() const {
	int value = static_cast<int>(next_event_time_rg_->getNextValue());
	return (value < 1) ? 1 : value;
  }

  inline PriceType getInitialPrice() const {
	return static_cast<PriceType>(initial_price_rg_->getNextValue());
  }

  inline int getDirection() const {
	return (direction_rg_->getNextValue() < 0.5) ? -1 : 1;
  }

  inline Side getSide() const {
	return (side_rg_->getNextValue() < 0.5) ? Side::BID : Side::ASK;
  }

  inline int getTickMove() const {
	return static_cast<int>(tick_move_rg_->getNextValue());
  }

  inline int getMaxTickDiff() const {
	return max_tick_diff_;
  }

 private:
  std::unique_ptr<IRandomDistributionGenerator> next_event_time_rg_{};
  std::unique_ptr<IRandomDistributionGenerator> initial_price_rg_{};
  std::unique_ptr<IRandomDistributionGenerator> direction_rg_{};
  std::unique_ptr<IRandomDistributionGenerator> tick_move_rg_{};
  std::unique_ptr<IRandomDistributionGenerator> side_rg_{};
  int max_tick_diff_{};
};

struct InstrumentSimulationFactory {
  static std::unique_ptr<InstrumentSimulationModel> create(
	  const std::vector<std::string> &next_event_time_cfg,
	  const std::vector<std::string> &initial_price_cfg,
	  const std::vector<std::string> &direction_cfg,
	  const std::vector<std::string> &tick_move_cfg,
	  const int &max_tick_diff) {

	// a fair chance either to move bid/ask hardcoded
	std::vector<std::string> side_cfg{"uniform_real_distribution", "0", "1"};

	return std::unique_ptr<InstrumentSimulationModel>(new InstrumentSimulationModel(
		std::move(RandomDistributionGeneratorFactory::create(next_event_time_cfg)),
		std::move(RandomDistributionGeneratorFactory::create((initial_price_cfg))),
		std::move(RandomDistributionGeneratorFactory::create((direction_cfg))),
		std::move(RandomDistributionGeneratorFactory::create((tick_move_cfg))),
		std::move(RandomDistributionGeneratorFactory::create((side_cfg))),
		max_tick_diff));
  }
};
}