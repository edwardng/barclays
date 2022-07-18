#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "base/types.h"

namespace basket::pricer {

struct BasketConfiguration {
  BasketConfiguration() {}
  BasketConfiguration(const double &lastPriceThreshold, const double &midPriceThreshold)
	  : lastPriceThreshold_(lastPriceThreshold), midPriceThreshold_(midPriceThreshold) {}

  double lastPriceThreshold_{0};
  double midPriceThreshold_{0};
};

class BasketPriceData {
 public:

  BasketPriceData() = default;

  BasketPriceData(const std::string &basket_name, const int &basket_id, const BasketConfiguration &basketConfiguration)
	  : basket_name_(basket_name), basket_id_(basket_id), basket_configuration_(basketConfiguration) {}

  BasketPriceData(const BasketPriceData &) = default;

  BasketPriceData &operator=(BasketPriceData &) = default;

  BasketPriceData(BasketPriceData &&) noexcept = default;

  BasketPriceData &operator=(BasketPriceData &&) noexcept = default;

  ~BasketPriceData() = default;

  [[nodiscard]] const std::string &getBasketName() {
	return basket_name_;
  }

  [[nodiscard]] const int getBasketId() {
	return basket_id_;
  }

  void setInstrumentWeight(const int &symbol_id, const double &weight);

  void setBasketToReady();

  [[nodiscard]] double getInstrumentWeighting(const int &symbol_id) const;

  [[nodiscard]] bool isReady() const {
	return is_ready_;
  };

  [[nodiscard]] const std::vector<double> &getAllWeights() const {
	return weighting_;
  };

  void setBidPrice(const PriceType &price);

  void setAskPrice(const PriceType &price);

  void setLastPrice(const PriceType &price);

  [[nodiscard]] PriceType getAskPrice() const {
	return ask_price_;
  }

  [[nodiscard]] PriceType getBidPrice() const {
	return bid_price_;
  }

  [[nodiscard]] PriceType getMidPrice() const {
	return mid_price_;
  }

  [[nodiscard]] PriceType getLastPrice() const {
	return last_price_;
  }

  [[nodiscard]] const BasketConfiguration &getBasketConfiguration() const {
	return basket_configuration_;
  }

 private:
  void updateMidPrice();

  PriceType bid_price_{0};
  PriceType ask_price_{0};
  PriceType mid_price_{0};
  PriceType last_price_{0};

  BasketConfiguration basket_configuration_;

  int basket_id_{};
  bool is_ready_{false};

  const std::string basket_name_{};
  std::vector<double> weighting_{};
};

class BasketsComposition {
 public:

  BasketsComposition() = default;

  explicit BasketsComposition(
	  const std::string &basketInfoCsvPath,
	  const std::string &basketConfigCsvPath);

  BasketsComposition(const BasketsComposition &) = default;

  BasketsComposition(BasketsComposition &&) noexcept = default;

  BasketsComposition &operator=(const BasketsComposition &) = default;

  BasketsComposition &operator=(BasketsComposition &&) noexcept = default;

  ~BasketsComposition() = default;

  [[nodiscard]] int getInstrumentID(const std::string &instrumentName) const;

  [[nodiscard]] std::vector<std::string> getInstrumentList() const;

  [[nodiscard]] std::vector<BasketPriceData> &getBasketPriceData() {
	return baskets_price_data_;
  }

 private:
  std::vector<BasketPriceData> baskets_price_data_{};
  std::unordered_map<std::string, int> instrumentName_to_id_map_{};
  std::unordered_map<std::string, BasketConfiguration> basket_configs_;
};

}