#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <unordered_map>

#include "InstrumentSimulationModel.h"

#include "InstrumentPrice.h"
#include "IMarketDataProvider.h"
#include "TickEvent.h"

namespace basket::pricer {
struct GenerationData {
  std::unique_ptr<InstrumentSimulationModel> generation_model_{};
  InstrumentPrice instrumentPrice{};
};

class TickDataGenerator : public IMarketDataProvider {
 public:
  explicit TickDataGenerator(const std::string &csv_path);

  ~TickDataGenerator() = default;

  TickDataGenerator() = delete;

  TickDataGenerator(const TickDataGenerator &) = delete;

  TickDataGenerator &operator=(const TickDataGenerator &) = delete;

  TickDataGenerator(TickDataGenerator &&) noexcept = delete;

  TickDataGenerator &operator=(TickDataGenerator &&) noexcept = delete;

  void subscribe(CallbackFunc &&callback, std::vector<std::string> &&instrumentList) override;
  void run() override;

 private:

  void simulateInstrument(const std::string &instrumentName);

  InstrumentPrice produceNewPriceShape(
	  const GenerationData &data,
	  const std::string &instrumentName) const;

  void enqueueNewTickEvents(
	  const InstrumentPrice &newInstrumentPrice,
	  const GenerationData &generationData,
	  const std::string &instrumentName);

  std::uint64_t lastest_event_timestamp_{0};

  std::priority_queue<TickEvent, std::vector<TickEvent>, std::greater<TickEvent>> pq_{};

  std::unordered_map<std::string, GenerationData> instrument_model_{};

};
}