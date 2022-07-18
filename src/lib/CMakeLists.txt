include_directories(lib/include)

add_subdirectory(lib/util)
add_subdirectory(lib/marketdata)
add_subdirectory(lib/simulation)
add_subdirectory(lib/basketpricer)

add_subdirectory(app)


project(BasketPricer)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.74.0 REQUIRED)

add_library(BasketPricerLib
        ../include/Basket.h
        Basket.cpp
        ../include/BasketPricer.h
        BasketPricer.cpp)

target_link_libraries(SimulateBasketPricer BasketPricerLib CommonUtilLib MarketDataLib SimulationLib)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(SimulateBasketPricer ${Boost_LIBRARIES})
endif ()

