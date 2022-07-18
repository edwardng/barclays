#pragma once

#include <cstdint>
#include <string>

#include "base/types.h"

namespace basket::pricer {
enum class TickEventType : std::int32_t {
  BID,
  ASK,
  TRADE,
  INVALID
};

struct TickEvent {
  TickEvent() = default;

  TickEvent(const std::uint64_t &event_timestamp,
			const PriceType &price,
			const TickEventType &eventType,
			const std::string &instrumentName)
	  : event_timestamp_(event_timestamp), price_(price), eventType_(eventType),
		instrumentName_(instrumentName) {
  }

  std::uint64_t event_timestamp_{0};
  PriceType price_{0};

  TickEventType eventType_{TickEventType::INVALID};
  std::string instrumentName_{};

  friend bool operator<(const TickEvent &lhs, const TickEvent &rhs);

  friend bool operator>(const TickEvent &lhs, const TickEvent &rhs);

  friend bool operator<=(const TickEvent &lhs, const TickEvent &rhs);

  friend bool operator>=(const TickEvent &lhs, const TickEvent &rhs);
};
}