#include "TickEvent.h"

namespace basket::pricer {
bool operator<(const TickEvent &lhs, const TickEvent &rhs) {
  return lhs.event_timestamp_ < rhs.event_timestamp_;
}

bool operator>(const TickEvent &lhs, const TickEvent &rhs) {
  return lhs.event_timestamp_ > rhs.event_timestamp_;
}

bool operator<=(const TickEvent &lhs, const TickEvent &rhs) {
  return lhs.event_timestamp_ <= rhs.event_timestamp_;
}

bool operator>=(const TickEvent &lhs, const TickEvent &rhs) {
  return lhs.event_timestamp_ >= rhs.event_timestamp_;
}
}