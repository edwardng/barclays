#pragma once

#include "base/types.h"

namespace basket::pricer {
class InstrumentPrice {
 public:

  InstrumentPrice() = default;

  InstrumentPrice(const PriceType &bid_price,
				  const PriceType &ask_price,
				  const PriceType &last_price)
	  : bid_price_(bid_price), ask_price_(ask_price), last_price_(last_price) {
  }

  InstrumentPrice(const InstrumentPrice &) = default;

  InstrumentPrice(InstrumentPrice &&) noexcept = default;

  InstrumentPrice &operator=(const InstrumentPrice &) = default;

  InstrumentPrice &operator=(InstrumentPrice &&) noexcept = default;

  ~InstrumentPrice() = default;

  void setBidPrice(const PriceType &price) {
	bid_price_ = price;
  }

  void setAskPrice(const PriceType &price) {
	ask_price_ = price;
  }

  void setLastPrice(const PriceType &price) {
	last_price_ = price;
  }

  [[nodiscard]] PriceType getAskPrice() const {
	return ask_price_;
  }

  [[nodiscard]] PriceType getBidPrice() const {
	return bid_price_;
  }

  [[nodiscard]] PriceType getLastPrice() const {
	return last_price_;
  }

 private:

  PriceType bid_price_{0};
  PriceType ask_price_{0};
  PriceType last_price_{0};

};
}