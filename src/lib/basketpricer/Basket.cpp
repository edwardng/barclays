#include <algorithm>
#include <sstream>

#include <iostream>

#include "Basket.h"
#include "CSVReader.h"

namespace basket::pricer {
void BasketPriceData::setBidPrice(const PriceType &price) {
  bid_price_ = price;
  updateMidPrice();
}

void BasketPriceData::setAskPrice(const PriceType &price) {
  ask_price_ = price;
  updateMidPrice();
}

void BasketPriceData::setLastPrice(const PriceType &price) {
  last_price_ = price;
}

void BasketPriceData::updateMidPrice() {
  if (ask_price_ > 0 && bid_price_ > 0) {
	mid_price_ = (ask_price_ + bid_price_) / 2;
  }
}

void BasketPriceData::setInstrumentWeight(const int &symbol_id, const double &weight) {
  if (weighting_.size() <= symbol_id) weighting_.resize(symbol_id + 1);
  weighting_[symbol_id] = weight;
}

void BasketPriceData::setBasketToReady() {
  is_ready_ = true;
}

[[nodiscard]] double BasketPriceData::getInstrumentWeighting(const int &symbol_id) const {
  if (symbol_id >= weighting_.size()) return 0;
  return weighting_[symbol_id];
}

BasketsComposition::BasketsComposition(const std::string &basketInfoCsv, const std::string &basketConfigCsvPath) {

  // Basket Config
  {
	constexpr static std::string_view BASKET_ID = "Basket ID";
	constexpr static std::string_view LAST_PRICE_THRESHOLD = "LastPrice Threshold";
	constexpr static std::string_view MID_PRICE_THRESHOLD = "MidPrice Threshold";

	constexpr static int HEADER_ROW_INDEX = 0;

	CSVReader basketConfigCsvReader(basketConfigCsvPath);
	auto data = basketConfigCsvReader.getData();

	const auto &header_row = data[HEADER_ROW_INDEX];
	int basket_id_col{-1}, last_price_threshold_col{-1}, mid_price_threshold_col{-1};

	for (int i = 0; i < header_row.size(); i++) {
	  if (header_row[i] == BASKET_ID) {
		basket_id_col = i;
	  } else if (header_row[i] == LAST_PRICE_THRESHOLD) {
		last_price_threshold_col = i;
	  } else if (header_row[i] == MID_PRICE_THRESHOLD) {
		mid_price_threshold_col = i;
	  }
	}

	std::istringstream iss;

	if (basket_id_col >= 0 && last_price_threshold_col >= 0 && mid_price_threshold_col >= 0) {
	  for (int i = 1; i < data.size(); i++) {
		const auto &row = data[i];

		const auto &basketName = row[basket_id_col];

		double midPriceThreshold{0}, lastPriceThreshold{0};

		iss.clear();
		iss.str(row[last_price_threshold_col]);
		iss >> lastPriceThreshold;

		iss.clear();
		iss.str(row[mid_price_threshold_col]);
		iss >> midPriceThreshold;

		basket_configs_[basketName] = std::move(BasketConfiguration{lastPriceThreshold, midPriceThreshold});
	  }
	}
  }

  // Basket Items
  {
	constexpr static std::string_view BASKET_ID = "Basket ID";
	constexpr static std::string_view BASKET_ITEM_ID = "Basket Item ID";
	constexpr static std::string_view WEIGHT = "Weight";

	constexpr static int HEADER_ROW_INDEX = 0;

	CSVReader basketInfoCsvReader(basketInfoCsv);
	auto data = basketInfoCsvReader.getData();

	const auto &header_row = data[HEADER_ROW_INDEX];
	int basket_id_col{-1}, basket_item_id_col{-1}, item_weight_col{-1};

	for (int i = 0; i < header_row.size(); i++) {
	  if (header_row[i] == BASKET_ID) {
		basket_id_col = i;
	  } else if (header_row[i] == BASKET_ITEM_ID) {
		basket_item_id_col = i;
	  } else if (header_row[i] == WEIGHT) {
		item_weight_col = i;
	  }
	}

	std::istringstream iss;
	std::unordered_map<std::string, int> basket_name_to_id_map_{};

	if (basket_id_col >= 0 && basket_item_id_col >= 0 && item_weight_col >= 0) {
	  for (int i = 1; i < data.size(); i++) {
		const auto &row = data[i];

		iss.clear();
		iss.str(row[item_weight_col]);
		double instrument_weight_in_basket{0};
		iss >> instrument_weight_in_basket;

		int basket_index_position = 0;
		const auto &basket_name = row[basket_id_col];
		{
		  auto itr = basket_name_to_id_map_.find(basket_name);
		  if (itr == basket_name_to_id_map_.end()) {
			basket_index_position = basket_name_to_id_map_.size();
			basket_name_to_id_map_[basket_name] = basket_index_position;
		  } else {
			basket_index_position = itr->second;
		  }
		}

		int instrument_index_position = 0;
		const auto &instrumentName = row[basket_item_id_col];
		{
		  auto itr = instrumentName_to_id_map_.find(instrumentName);
		  if (itr == instrumentName_to_id_map_.end()) {
			instrument_index_position = instrumentName_to_id_map_.size();
			instrumentName_to_id_map_[instrumentName] = instrument_index_position;
		  } else {
			instrument_index_position = itr->second;
		  }
		}

		if (basket_index_position >= baskets_price_data_.size()) {
		  BasketConfiguration basketConfig;
		  auto itr = basket_configs_.find(basket_name);
		  if (itr != basket_configs_.end()) basketConfig = itr->second;

		  BasketPriceData basketInfo(basket_name, basket_index_position, basketConfig);
		  basketInfo.setInstrumentWeight(instrument_index_position, instrument_weight_in_basket);

		  baskets_price_data_.push_back(std::move(basketInfo));
		} else {
		  BasketPriceData &basketInfo = baskets_price_data_[basket_index_position];
		  basketInfo.setInstrumentWeight(instrument_index_position, instrument_weight_in_basket);
		}
	  }
	}

  }
}

[[nodiscard]] int BasketsComposition::getInstrumentID(const std::string &instrumentName) const {
  auto itr = instrumentName_to_id_map_.find(instrumentName);
  if (itr != instrumentName_to_id_map_.end()) return itr->second;
  return -1;
}

[[nodiscard]] std::vector<std::string> BasketsComposition::getInstrumentList() const {
  std::vector<std::string> instrumentList;

  std::transform(instrumentName_to_id_map_.begin(), instrumentName_to_id_map_.end(),
				 std::inserter(instrumentList, instrumentList.begin()),
				 [](const auto &itr) {
				   return itr.first;
				 });

  return instrumentList;
}
}