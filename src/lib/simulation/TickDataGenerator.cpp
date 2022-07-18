#include "TickDataGenerator.h"

#include <iostream>
#include <sstream>
#include <utility>

#include "CSVReader.h"

#include "base/double_comparison.h"

namespace basket::pricer {
TickDataGenerator::TickDataGenerator(const std::string &csv_path) {

  constexpr static int ROW_INDEX_INSTRUMENT_NAME = 0;
  constexpr static int ROW_INDEX_NEXT_PRICE_CFG = 1;
  constexpr static int ROW_INDEX_INITIAL_PRICE_CFG = 2;
  constexpr static int ROW_INDEX_DIRECTION_CFG = 3;
  constexpr static int ROW_INDEX_NUMBER_OF_TICKS_CFG = 4;
  constexpr static int ROW_INDEX_MAX_TICKS_DIFF = 5;
  constexpr static int ROW_COUNT_TOTAL = 6;

  CSVReader csvReader(csv_path);
  auto data = csvReader.getData();

  std::istringstream iss;

  for (int i = 0; i < data.size(); i += ROW_COUNT_TOTAL) {
	const auto &instrumentName = data[i][ROW_INDEX_INSTRUMENT_NAME];

	const auto &next_event_price_cfg = data[i + ROW_INDEX_NEXT_PRICE_CFG];
	const auto &initial_price_cfg = data[i + ROW_INDEX_INITIAL_PRICE_CFG];
	const auto &direction_cfg = data[i + ROW_INDEX_DIRECTION_CFG];
	const auto &number_of_ticks_cfg = data[i + ROW_INDEX_NUMBER_OF_TICKS_CFG];

	int max_tick_diff{0};
	iss.clear();
	iss.str(data[i + ROW_INDEX_MAX_TICKS_DIFF][0]);
	iss >> max_tick_diff;

	GenerationData generation_data;
	generation_data.generation_model_ = std::move(InstrumentSimulationFactory::create(
		next_event_price_cfg,
		initial_price_cfg,
		direction_cfg,
		number_of_ticks_cfg,
		max_tick_diff
	));
	generation_data.instrumentPrice = InstrumentPrice{};

	instrument_model_[instrumentName] = std::move(generation_data);
  }
}

void TickDataGenerator::subscribe(CallbackFunc &&callback, std::vector<std::string> &&instrumentList) {
  callback_ = std::move(callback);

  for (const auto &instrumentName : instrumentList) {
	simulateInstrument(instrumentName);
  }
}

void TickDataGenerator::run() {

  std::set<std::string> instruments_with_events{};
  std::uint64_t prev_event_clock_tick{0};

  while (!pq_.empty()) {
	const auto &tickEvent = pq_.top();

	if (tickEvent.event_timestamp_ < lastest_event_timestamp_) {
	  // we reached the end of the world - timestamp increment from uint64_t max back to 0
	  return;
	}

	lastest_event_timestamp_ = tickEvent.event_timestamp_;
	callback_(tickEvent);
	instruments_with_events.insert(tickEvent.instrumentName_);

	pq_.pop();

	if (tickEvent.event_timestamp_ > prev_event_clock_tick || pq_.empty()) {
	  for (const auto &instrumentName : instruments_with_events) {
		simulateInstrument(instrumentName);
	  }
	  instruments_with_events.clear();
	  prev_event_clock_tick = tickEvent.event_timestamp_;
	}
  }
}

void TickDataGenerator::simulateInstrument(const std::string &instrumentName) {
  auto itr = instrument_model_.find(instrumentName);
  if (itr == instrument_model_.end()) [[unlikely]] {
	std::ostringstream oss;
	oss << "Unexpected instrument - instrument " << instrumentName << " simulation model not specified?";
	throw std::invalid_argument(oss.str());
  }

  GenerationData &generationData = itr->second;
  InstrumentPrice newInstrumentPrice = produceNewPriceShape(generationData, instrumentName);

  enqueueNewTickEvents(newInstrumentPrice, generationData, instrumentName);
  generationData.instrumentPrice = newInstrumentPrice;
}

InstrumentPrice TickDataGenerator::produceNewPriceShape(
	const GenerationData &generationData,
	const std::string &instrumentName) const {
  static constexpr PriceType ticksize = 0.01;

  auto &currentInstrumentPrice = generationData.instrumentPrice;
  auto newInstrumentPrice = currentInstrumentPrice;

  const PriceType &currBidPrice = currentInstrumentPrice.getBidPrice();
  const PriceType &currAskPrice = currentInstrumentPrice.getAskPrice();

  const auto &generationMode = generationData.generation_model_;

  if (!double_equal(currBidPrice, 0) && !double_equal(currAskPrice, 0)) {

	int maxTickMove = generationMode->getMaxTickDiff();
	int newTickDiff{0};

	do {
	  newInstrumentPrice = currentInstrumentPrice;

	  int tickMove = generationMode->getTickMove();
	  int direction = generationMode->getDirection();
	  auto side = generationMode->getSide();
	  PriceType delta = ticksize * tickMove * direction;

	  if (side == Side::BID) {
		PriceType newPrice = currBidPrice + delta;
		newInstrumentPrice.setBidPrice(newPrice);

		if (newPrice > 0 &&
			(newInstrumentPrice.getBidPrice() > newInstrumentPrice.getAskPrice() ||
				double_equal(newInstrumentPrice.getAskPrice(), newInstrumentPrice.getBidPrice()))) {
		  // bid crossed ask -> traded up and move price up
		  newInstrumentPrice.setAskPrice(newPrice + (generationMode->getTickMove() * ticksize));
		}
	  } else if (side == Side::ASK) {
		PriceType newPrice = currAskPrice + delta;
		newInstrumentPrice.setAskPrice(newPrice);

		if (newPrice > 0 &&
			(newInstrumentPrice.getBidPrice() > newInstrumentPrice.getAskPrice() ||
				double_equal(newInstrumentPrice.getAskPrice(), newInstrumentPrice.getBidPrice()))) {
		  // ask crossed bid -> traded down and move price down
		  newInstrumentPrice.setBidPrice(newPrice - (generationMode->getTickMove() * ticksize));
		}
	  }

	  newTickDiff =
		  static_cast<int>(std::fabs(newInstrumentPrice.getAskPrice() - newInstrumentPrice.getBidPrice()) / ticksize);

	} while (newTickDiff == 0 ||
		newInstrumentPrice.getAskPrice() < ticksize ||
		newInstrumentPrice.getBidPrice() < ticksize ||
		double_equal(newInstrumentPrice.getAskPrice(), ticksize) ||
		double_equal(newInstrumentPrice.getBidPrice(), ticksize) ||
		maxTickMove < newTickDiff);
  } else if (double_equal(currAskPrice, 0))  [[unlikely]] {

	PriceType newPrice{0};
	if (double_equal(currBidPrice, 0))
	  newPrice = generationMode->getInitialPrice();
	else
	  newPrice = currBidPrice + (generationMode->getTickMove() * ticksize);

	newInstrumentPrice.setAskPrice(newPrice);

  } else if (double_equal(currBidPrice, 0)) [[unlikely]] {

	PriceType newPrice{0};
	if (double_equal(currAskPrice, 0))
	  newPrice = generationMode->getInitialPrice();
	else
	  newPrice = currAskPrice - (generationMode->getTickMove() * ticksize);

	newInstrumentPrice.setBidPrice(newPrice);

  }

  return newInstrumentPrice;
}

void TickDataGenerator::enqueueNewTickEvents(
	const InstrumentPrice &newInstrumentPrice,
	const GenerationData &generationData,
	const std::string &instrumentName) {

  const auto &prevInstrumentPrice = generationData.instrumentPrice;
  const auto &generationMode = generationData.generation_model_;
  const std::uint64_t nextEventTime = lastest_event_timestamp_ + generationMode->getNextEventTime();

  if (!double_equal(newInstrumentPrice.getAskPrice(), prevInstrumentPrice.getAskPrice())) {
	pq_.emplace(nextEventTime,
				newInstrumentPrice.getAskPrice(),
				TickEventType::ASK,
				instrumentName);
  }
  if (!double_equal(newInstrumentPrice.getBidPrice(), prevInstrumentPrice.getBidPrice())) {
	pq_.emplace(nextEventTime,
				newInstrumentPrice.getBidPrice(),
				TickEventType::BID,
				instrumentName);
  }

  if (!double_equal(prevInstrumentPrice.getBidPrice(), 0) &&
	  !double_equal(prevInstrumentPrice.getAskPrice(), 0)) {

	PriceType tradePrice{0};

	if (newInstrumentPrice.getBidPrice() > prevInstrumentPrice.getAskPrice() ||
		double_equal(newInstrumentPrice.getBidPrice(), prevInstrumentPrice.getAskPrice())) {
	  tradePrice = prevInstrumentPrice.getAskPrice();
	} else if (newInstrumentPrice.getAskPrice() < prevInstrumentPrice.getBidPrice() ||
		double_equal(newInstrumentPrice.getAskPrice(), prevInstrumentPrice.getBidPrice())) {
	  tradePrice = prevInstrumentPrice.getBidPrice();
	}
	if (!double_equal(tradePrice, 0)) {
	  pq_.emplace(nextEventTime,
				  tradePrice,
				  TickEventType::TRADE,
				  instrumentName);
	}
  }

}
}