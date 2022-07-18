#pragma once

#include <queue>
#include <memory>
#include <condition_variable>

#include "base/types.h"
#include "Basket.h"
#include "IMarketDataProvider.h"
#include "TickEvent.h"

namespace basket::pricer {

class InstrumentPrice;

struct ThresholdEvent {
	int basket_id_;
	TickEventType event_type_;
	PriceType prev_price_;
	PriceType new_price_;
	double delta_pct_;
};

class BasketPricer {
 public:

  BasketPricer(const BasketsComposition &basketComposition,
			   std::shared_ptr<IMarketDataProvider> marketDataProvider);

  BasketPricer() = delete;

  BasketPricer(const BasketPricer &) = delete;

  BasketPricer(BasketPricer &&) noexcept = default;

  BasketPricer &operator=(const BasketPricer &) = delete;

  BasketPricer &operator=(BasketPricer &&) noexcept = default;

  ~BasketPricer() = default;

  void initMarketDataSubscription();

 private:

  // We may want to make it configurable?
  constexpr static int THRESHOLD_MESSAGES_SIZE = 30;

  BasketsComposition basketComposition_;

  std::shared_ptr<IMarketDataProvider> marketDataProvider_{};
  std::vector<InstrumentPrice> instrument_prices_{};


  std::mutex threshold_message_mutex_{};
  std::condition_variable threshold_message_cv_{};
  std::vector<ThresholdEvent> threshold_messages_{};

};

}