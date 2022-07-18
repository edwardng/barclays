#pragma once

#include <functional>
#include <set>
#include <string>

namespace basket::pricer {
class TickEvent;

class IMarketDataProvider {
 public:
  using CallbackFunc = std::function<void(const TickEvent &tickEvent)>;

  virtual void subscribe(CallbackFunc &&callback, std::vector<std::string> &&instrumentList) = 0;
  virtual void run() = 0;

 protected:
  CallbackFunc callback_{};
};
}