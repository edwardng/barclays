#include <utility>

#include <future>
#include <thread>
#include <iostream>
#include <sstream>

#include "BasketPricer.h"
#include "TickDataGenerator.h"

#include "base/double_comparison.h"

namespace basket::pricer {
BasketPricer::BasketPricer(const BasketsComposition &basketComposition,
						   std::shared_ptr<IMarketDataProvider> marketDataProvider)
	: basketComposition_(basketComposition), marketDataProvider_(marketDataProvider) {
}

void BasketPricer::initMarketDataSubscription() {

  auto instrumentList = basketComposition_.getInstrumentList();
  std::transform(instrumentList.begin(), instrumentList.end(),
				 std::back_inserter(instrument_prices_),
				 [](const auto &itr) {
				   return InstrumentPrice{};
				 });

  auto init_basket_data_when_ready = [this](BasketPriceData &basket_price_data) {
	bool is_basket_ready{true};

	const auto &weights = basket_price_data.getAllWeights();
	for (int i = 0; i < weights.size(); i++) {
	  if (weights[i] > 0) {
		const auto &instrument_price = instrument_prices_[i];
		if (double_equal(instrument_price.getAskPrice(), 0) ||
			double_equal(instrument_price.getBidPrice(), 0) ||
			double_equal(instrument_price.getLastPrice(), 0)) {
		  is_basket_ready = false;
		  break;
		}
	  }
	}

	if (is_basket_ready) {
	  // set initial prices...
	  PriceType ask_weighted{0}, bid_weighted{0}, last_weighted{0};

	  for (int i = 0; i < weights.size(); i++) {
		if (weights[i] > 0) {
		  const auto &instrument_price = instrument_prices_[i];
		  ask_weighted += instrument_price.getAskPrice() * weights[i];
		  bid_weighted += instrument_price.getBidPrice() * weights[i];
		  last_weighted += instrument_price.getLastPrice() * weights[i];
		}
	  }

	  basket_price_data.setAskPrice(ask_weighted);
	  basket_price_data.setBidPrice(bid_weighted);
	  basket_price_data.setLastPrice(last_weighted);
	  basket_price_data.setBasketToReady();
	}
  };

  // *** OnTickUpdate - Critical Fast Path Start ***
  auto onTickUpdate = [this, init_basket_data_when_ready]
	  (const TickEvent &tickEvent) {

	if (tickEvent.eventType_ == TickEventType::INVALID) [[unlikely]] {
	  throw std::logic_error("Invalid TickEvent Type encountered!");
	}

	// system generated instrument id starting from 0
	auto instrumentId = basketComposition_.getInstrumentID(tickEvent.instrumentName_);
	if (instrumentId < 0) [[unlikely]] return;

	auto &instrument_price = instrument_prices_[instrumentId];

	PriceType instrument_prev_price{0};

	if (tickEvent.eventType_ == TickEventType::ASK) {
	  instrument_prev_price = instrument_price.getAskPrice();
	  instrument_price.setAskPrice(tickEvent.price_);
	} else if (tickEvent.eventType_ == TickEventType::BID) {
	  instrument_prev_price = instrument_price.getBidPrice();
	  instrument_price.setBidPrice(tickEvent.price_);
	} else if (tickEvent.eventType_ == TickEventType::TRADE) {
	  instrument_prev_price = instrument_price.getLastPrice();
	  instrument_price.setLastPrice(tickEvent.price_);
	}

	// for each basket


	for (auto &basket_price_data : basketComposition_.getBasketPriceData()) {
	  if (!basket_price_data.isReady()) [[unlikely]] {
		// Slowness in critical path only happens when market starts
		init_basket_data_when_ready(basket_price_data);
		continue;
	  }

	  const auto &weight = basket_price_data.getInstrumentWeighting(instrumentId);
	  if (!double_equal(weight, 0)) {

		auto basket_weighted_delta = (tickEvent.price_ - instrument_prev_price) * weight;

		if (tickEvent.eventType_ == TickEventType::TRADE) {
		  const PriceType prev_last_price = basket_price_data.getLastPrice();
		  const PriceType new_last_price = prev_last_price + basket_weighted_delta;
		  basket_price_data.setLastPrice(new_last_price);

		  double delta_pct = (std::fabs(new_last_price - prev_last_price) / prev_last_price) * 100.0;
		  if (delta_pct > basket_price_data.getBasketConfiguration().lastPriceThreshold_) {
			std::lock_guard<std::mutex> lg(threshold_message_mutex_);
			threshold_messages_.push_back({
											  basket_price_data.getBasketId(),
											  tickEvent.eventType_,
											  prev_last_price,
											  new_last_price,
											  delta_pct
										  });
			threshold_message_cv_.notify_one();
		  }

		} else {
		  const PriceType prev_mid_price = basket_price_data.getMidPrice();

		  if (tickEvent.eventType_ == TickEventType::ASK) {
			basket_price_data.setAskPrice(basket_price_data.getAskPrice() + basket_weighted_delta);
		  } else if (tickEvent.eventType_ == TickEventType::BID) {
			basket_price_data.setBidPrice(basket_price_data.getBidPrice() + basket_weighted_delta);
		  }

		  const auto new_mid_price = basket_price_data.getMidPrice();
		  double delta_pct = (std::fabs(new_mid_price - prev_mid_price) / prev_mid_price) * 100.0;

		  if (delta_pct > basket_price_data.getBasketConfiguration().midPriceThreshold_) {
			std::lock_guard<std::mutex> lg(threshold_message_mutex_);
			threshold_messages_.push_back({
											  basket_price_data.getBasketId(),
											  tickEvent.eventType_,
											  prev_mid_price,
											  new_mid_price,
											  delta_pct
										  });
			threshold_message_cv_.notify_one();
		  }
		}
	  }
	}
  };
  // *** Critical Fast Path Complete ***

  threshold_messages_.reserve(THRESHOLD_MESSAGES_SIZE);

  marketDataProvider_->subscribe(onTickUpdate,
								 std::move(basketComposition_.getInstrumentList()));

  std::thread threshold_breach_printer([this] {
	while (true) {
	  std::vector<ThresholdEvent> outstanding_messages_;
	  outstanding_messages_.reserve(THRESHOLD_MESSAGES_SIZE);

	  {
		std::unique_lock<std::mutex> ul(threshold_message_mutex_);
		threshold_message_cv_.wait(ul, [this] { return !threshold_messages_.empty(); });
		outstanding_messages_.swap(threshold_messages_);
	  }

	  std::ostringstream oss;

	  for (decltype(outstanding_messages_.size()) size = 0; size < outstanding_messages_.size(); size++) {
		const auto &msg = outstanding_messages_[size];

		const auto &basketName = basketComposition_.getBasketPriceData()[msg.basket_id_].getBasketName();
		oss << basketName;

		if (msg.event_type_ == TickEventType::TRADE) {
		  oss << " PrevLastPrice " << msg.prev_price_
			  << " NewLastPrice " << msg.new_price_;
		} else {
		  oss << " PrevMidPrice " << msg.prev_price_
			  << " NewMidPrice " << msg.new_price_;
		}
		oss << " DeltaPct " << msg.delta_pct_ << std::endl;

		std::cout << oss.str();
	  }
	}
  });
  threshold_breach_printer.detach();

}
}