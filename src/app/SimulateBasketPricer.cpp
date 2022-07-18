#include <iostream>

#include "Basket.h"
#include "BasketPricer.h"
#include "TickDataGenerator.h"

int main(int argc, char *argv[]) {
  if (argc < 4) {
	std::cerr
		<< "missing program arguments" << std::endl
		<< "expected: " << argv[0] << " " << "path_to_basket_data.csv path_to_basket_config.cfg path_to_instrument_simulation.cfg"
		<< std::endl;
	return 1;
  }

  try {
	basket::pricer::BasketsComposition basket_composition(argv[1], argv[2]);

	std::shared_ptr<basket::pricer::IMarketDataProvider> marketDataProvider(
		new basket::pricer::TickDataGenerator(argv[3]));

	basket::pricer::BasketPricer pricer(basket_composition, marketDataProvider);
	pricer.initMarketDataSubscription();

	marketDataProvider->run();
  }
  catch (const std::exception &e) {
	std::cerr << e.what();
  }
}